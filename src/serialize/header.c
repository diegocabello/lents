#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gents.h"

/**
 * Serializes the file header into a newly allocated buffer
 * 
 * Header format (32 bytes total):
 * - Magic number "GENTS" (5 bytes)
 * - Version number (1 byte)
 * - Max forest size (1 byte)
 * - Max children (1 byte)
 * - Max depth (1 byte)
 * - Padding (23 bytes of zeros to make total of 32 bytes)
 * 
 * @return Pointer to allocated buffer containing header (32 bytes), or NULL on failure
 *         Caller is responsible for freeing this memory
 */
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
