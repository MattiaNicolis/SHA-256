#ifndef SHA256_UTILS_H
#define SHA256_UTILS_H

#include<stdint.h>

void digest_file(const char *filename, uint8_t *hash);

#endif