#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gents.h"

/**
 * Serializes the file header into a newly allocated buffer
 * 
 * Header format (16 bytes total):
 * - Magic number "GENTS" (5 bytes)
 * - Version number (1 byte)
 * - Max forest size (1 byte)
 * - Max children (1 byte)
 * - Max depth (1 byte)
 * - Padding (7 bytes of zeros)
 * 
 * @return Pointer to allocated buffer containing header (16 bytes), or NULL on failure
 *         Caller is responsible for freeing this memory
 */
unsigned char* serialize_header() {
    unsigned char* buffer = (unsigned char*)malloc(16);
    if (!buffer) return NULL;
    
    int offset = 0;
    
    // Write magic number (5 bytes)
    memcpy(buffer + offset, MAGIC_NUMBER, strlen(MAGIC_NUMBER));
    offset += strlen(MAGIC_NUMBER);
    buffer[offset++] = VERSION_NUMBER;
    buffer[offset++] = MAX_FOREST_SIZE;
    buffer[offset++] = MAX_CHILDREN;
    buffer[offset++] = MAX_DEPTH;
    
    // Write padding (7 bytes of zeros)
    memset(buffer + offset, 0, 7);
    
    return buffer;
}
