#ifndef COMMON_H
#define COMMON_H

#include<stdint.h>

// Percorso noto della FIFO pubblica del server
#define SERVER_FIFO "/tmp/sha256_server_fifo"

#define MAX_PATH 256
#define HASH_SIZE 65 // 64 caratteri esadecimali + 1 terminatore '\0'

// Tipi di richiesta supportati
typedef enum {
    REQ_COMPUTE,                // richiesta di calcolo hash
    REQ_QUERY_CACHE,            // richiesta di dump della cache
    REQ_EXIT                    // richiesta di spegnimento (opzionale)
} req_type_t;

// Struttura inviata dal client al server
typedef struct {
    req_type_t type;             // tipo operazione
    char file_path[MAX_PATH];    // percorso del file da analizzare
    char client_fifo[MAX_PATH];  // indirizzo di ritorno (FIFO privata)
    long file_size;              // dimensione file (usata per ordinamento SJF)
} request_t;

// Struttura inviata dal server al client
typedef struct {
    char hash[HASH_SIZE];        // risultato SHA-256 (o messaggio errore)
    int status;                  // 0 = OK, altro = ERRORE
    int from_cache;              // 1 = recuperato da cache, 0 = calcolato ora
} response_t;

#endif