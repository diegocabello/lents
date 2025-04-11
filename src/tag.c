#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

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

// Add a tag to a file if it doesn't already have one
// Returns 1 on success, 0 on failure
int add_tag(const char *filename, const char *attr_name) {
    char attr_value[256] = {0};
    
    // Check if file exists
    if (access(filename, F_OK) != 0) {
        fprintf(stderr, "File %s does not exist\n", filename);
        return 0;
    }
    
    // Check if the extended attribute already exists
    ssize_t attr_size = getxattr(filename, attr_name, attr_value, sizeof(attr_value) - 1);
    
    if (attr_size > 0) {
        // Attribute exists, file is already tagged
        printf("File already has ID: %s\n", attr_value);
        return 1;
    } else {
        // Attribute doesn't exist, generate and add it
        char *random_id = generate_random_id();
        if (random_id == NULL) {
            fprintf(stderr, "Failed to generate random ID\n");
            return 0;
        }
        
        if (setxattr(filename, attr_name, random_id, strlen(random_id), 0) != 0) {
            perror("Failed to set extended attribute");
            free(random_id);
            return 0;
        }
        
        printf("Added ID %s to file %s\n", random_id, filename);
        free(random_id);
        return 1;
    }
}