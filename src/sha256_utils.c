#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include "../include/sha256_utils.h"

// Funzione wrapper per calcolare l'hash SHA256 di un file
void digest_file(const char *filename, uint8_t *hash) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    char buffer[32];      // buffer di lettura
    int file = open(filename, O_RDONLY);

    // Se il file non si apre, la funzione ritorna
    if(file == -1) return; 

    ssize_t bR;
    // Legge il file a blocchi e aggiorna l'hash parziale
    while((bR = read(file, buffer, 32)) > 0){
        SHA256_Update(&ctx, (uint8_t *)buffer, bR);
    }

    // Genera l'hash finale
    SHA256_Final(hash, &ctx);
    close(file);
}