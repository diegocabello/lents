
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "gents.h"

#define HASH_BYTE_LENGTH 7
#define MAGIC_NUMBER "GENTS"
#define VERSION_NUMBER 1
#define MAX_FOREST_SIZE 23
#define MAX_CHILDREN 15
#define MAX_NAME_CHAR 44
#define MAX_DEPTH 8

unsigned char* serialize_header() {
    unsigned char* buffer = (unsigned char*)malloc(32); // Total size 32 bytes
    if (!buffer) return NULL;
    int offset = 0;
    
    // Write magic number (5 bytes)
    memcpy(buffer + offset, MAGIC_NUMBER, strlen(MAGIC_NUMBER));
    offset += strlen(MAGIC_NUMBER);
    
    // Write version and other fields (4 bytes total)
    buffer[offset++] = VERSION_NUMBER;
    buffer[offset++] = MAX_FOREST_SIZE;
    buffer[offset++] = MAX_CHILDREN;
    buffer[offset++] = MAX_DEPTH;
    
    // Write padding (23 bytes of zeros)
    // This gives us 5+4+23=32 bytes total
    memset(buffer + offset, 0, 23);
    
    return buffer;
}


