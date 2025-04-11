#include <string.h>

int is_alias(const char *operation, const char *commands[]) {
    for (int i = 0; commands[i] != NULL; i++) {
        if (strcmp(operation, commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Generate a random ID using /dev/urandom and convert to hex
char* generate_random_id() {
    FILE *f = fopen("/dev/urandom", "rb");
    if (f == NULL) {
        perror("Failed to open /dev/urandom");
        return NULL;
    }
    
    // Read 7 random bytes
    unsigned char bytes[7];
    size_t read_bytes = fread(bytes, 1, 7, f);
    fclose(f);
    
    if (read_bytes != 7) {
        fprintf(stderr, "Failed to read enough random bytes\n");
        return NULL;
    }
    
    // Convert to hex string (each byte becomes 2 hex chars, plus null terminator)
    char* hex_id = malloc(15);  // 7 bytes * 2 chars per byte + null terminator
    if (hex_id == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < 7; i++) {
        sprintf(hex_id + (i * 2), "%02x", bytes[i]);
    }
    hex_id[14] = '\0';
    
    return hex_id;
}