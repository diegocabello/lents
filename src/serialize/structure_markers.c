#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gents.h"

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


unsigned char* serialize_delimiter() {
    unsigned char* buffer = (unsigned char*)malloc(32); // Total size 32 bytes
    if (!buffer) return NULL;
    
    // Fill the entire buffer with the value 0x44 (binary 01000100)
    memset(buffer, 0x90, 32);
    
    return buffer;
}