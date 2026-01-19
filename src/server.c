#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "../include/common.h"
#include "../include/sha256_utils.h"

#define MAX_THREADS 4

// --- STRUTTURE DATI ---
typedef struct request_node {
    request_t data;
    struct request_node *next;
} request_node_t;

typedef struct cache_node {
    char path[MAX_PATH];
    char hash[HASH_SIZE];
    struct cache_node *next;
} cache_node_t;

// --- VARIABILI GLOBALI ---
request_node_t *request_queue = NULL;
cache_node_t *hash_cache = NULL;

pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t request_cond = PTHREAD_COND_INITIALIZER;

/**
 * Funzione: enqueue_request
 * Scopo: inserisce una richiesta nella coda.
 * Logica: implementa lo scheduling SJF (Shortest Job First).
 * La lista viene mantenuta ordinata in base a req.file_size.
 */
void enqueue_request(request_t req) {
    request_node_t *new_node = malloc(sizeof(request_node_t));
    new_node->data = req;
    new_node->next = NULL;

    // INIZIO SEZIONE CRITICA
    pthread_mutex_lock(&server_mutex);

    if(!request_queue || req.file_size < request_queue->data.file_size) {
        new_node->next = request_queue;
        request_queue = new_node;
    } else {
        request_node_t *curr = request_queue;
        while(curr->next && curr->next->data.file_size <= req.file_size) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }

    pthread_cond_signal(&request_cond);
    
    // FINE SEZIONE CRITICA
    pthread_mutex_unlock(&server_mutex);
}

/**
 * Funzione: worker_thread
 * Scopo: ciclo di vita dei thread operai.
 * Preleva richiesta -> Controlla Cache -> Calcola -> Risponde.
 */
void *worker_thread(void *arg) {
    while(1) {
        // 1. PRELIEVO RICHIESTA
        pthread_mutex_lock(&server_mutex);

        while(!request_queue) {
            pthread_cond_wait(&request_cond, &server_mutex);
        }

        request_node_t *node = request_queue;
        request_queue = request_queue->next;
        pthread_mutex_unlock(&server_mutex);

        request_t req = node->data;
        free(node);

        response_t res;
        memset(&res, 0, sizeof(response_t));
        res.from_cache = 0;
        res.status = 0;

        // 2. CONTROLLO CACHE
        pthread_mutex_lock(&server_mutex);
        cache_node_t *c = hash_cache;
        while(c) {
            if(strcmp(c->path, req.file_path) == 0) {
                strcpy(res.hash, c->hash);
                res.from_cache = 1;
                break;
            }
            c = c->next;
        }
        pthread_mutex_unlock(&server_mutex);

        // 3. CALCOLO DELL'HASH
        if(!res.from_cache) {
            if(access(req.file_path, F_OK) != 0) {
                printf("Errore: File %s non trovato.\n", req.file_path);
                strcpy(res.hash, "ERRORE_FILE_NOT_FOUND");
            } else {
                uint8_t raw_hash[32];
                digest_file(req.file_path, raw_hash);

                for(int i=0; i<32; i++)
                    sprintf(res.hash + (i * 2), "%02x", raw_hash[i]);
                res.hash[64] = '\0';

                // 4. AGGIORNAMENTO CACHE
                pthread_mutex_lock(&server_mutex);
                cache_node_t *new_cache = malloc(sizeof(cache_node_t));
                strncpy(new_cache->path, req.file_path, MAX_PATH);
                strcpy(new_cache->hash, res.hash);
                new_cache->next = hash_cache;
                hash_cache = new_cache;
                pthread_mutex_unlock(&server_mutex);
            }
        }

        // 5. INVIO RISPOSTA AL CLIENT
        int cfd = open(req.client_fifo, O_WRONLY);
        if(cfd != -1) {
            write(cfd, &res, sizeof(response_t));
            close(cfd);
            printf("Risposta inviata a: %s (Hash: %.10s...)\n", req.client_fifo, res.hash);
        } else {
            perror("Errore apertura FIFO client");
        }
    }
    return NULL;
}

void handle_shutdown(int sig) {
    printf("\nSpegnimento del server in corso...\n");
    unlink(SERVER_FIFO); // rimuove il file della FIFO pubblica
    exit(0);
}

int main() {
    unlink(SERVER_FIFO);

    // Crea la FIFO pubblica
    if(mkfifo(SERVER_FIFO, 0666) == -1) {
        perror("Errore creazione FIFO server");
        exit(1);
    }

    // Creazione del thread pool
    pthread_t pool[MAX_THREADS];
    for(int i=0; i<MAX_THREADS; i++)
        pthread_create(&pool[i], NULL, worker_thread, NULL);
    
    printf("Server avviato. In ascolto su %s...\n", SERVER_FIFO);

    // Apertura FIFO
    int sfd = open(SERVER_FIFO, O_RDWR);
    if(sfd == -1) {
        perror("Errore apertura FIFO server");
        exit(1);
    }

    signal(SIGINT, handle_shutdown);

    while(1) {
        request_t req;
        ssize_t n = read(sfd, &req, sizeof(request_t));

        if(n > 0) {
            if(req.type == REQ_QUERY_CACHE) {
                printf("Ricevuta richiesta ispezione cache.\n");
                int cfd = open(req.client_fifo, O_WRONLY);
                if(cfd != -1) {
                    pthread_mutex_lock(&server_mutex);
                    
                    if(hash_cache == NULL) {
                        char *msg = "La cache è vuota.\n";
                        write(cfd, msg, strlen(msg));
                    } else {
                        cache_node_t *curr = hash_cache;
                        while(curr) {
                            char line[512];
                            snprintf(line, sizeof(line), "File: %s | Hash: %s\n", curr->path, curr->hash);
                            write(cfd, line, strlen(line));
                            curr = curr->next;
                        }
                    }
                    
                    pthread_mutex_unlock(&server_mutex);
                    close(cfd);
                }
            } else if(req.type == REQ_COMPUTE) {
                printf("Ricevuta richiesta calcolo per: %s\n", req.file_path);
                enqueue_request(req);
            }
        }
    }
    return 0;
}