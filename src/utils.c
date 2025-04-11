#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//!!THESE ALL HAVE TO BE TERMINATED WITH NULL ELSE THE PROGRAM WILL HAVE PROBLEMS!!
const char *REMOVE_ALIASES[] ={"remove", "rm", NULL};

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
    
    unsigned char bytes[7];     // Read 7 random bytes
    size_t read_bytes = fread(bytes, 1, 7, f);
    fclose(f);
    
    if (read_bytes != 7) {
        fprintf(stderr, "Failed to read enough random bytes\n");
        return NULL;
    }
    
    char* hex_id = malloc(15);  // 7 bytes * 2 chars per byte + null terminator // Convert to hex string (each byte becomes 2 hex chars, plus null terminator)
    if (hex_id == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < 7; i++) {
        sprintf(hex_id + (i * 2), "%02x", bytes[i]);
    }
    hex_id[14] = '\0';
    
    return hex_id;
}

int is_bidir_map_fx_operation (const char* operation) {
    if (strcmp(operation, "add") == 0) {
        return 1
    } else if (is_alias(operation, REMOVE_ALIASES)) {
        return 2
    } else if (strcmp(operation, "show") == 0) {
        return 3
    } else {
        return 0
    }
}

int check_file_exists_and_tagged(filename) {

    if (access(filename, F_OK) != 0) {     // Check if file exists
        fprintf(stderr, "File %s does not exist\n", filename);
        return 1;
    }
    
    ssize_t attr_size = getxattr(filename, attr_name, attr_value, sizeof(attr_value) - 1); // Check if the extended attribute already exists
    
    if (attr_size == 0) {
        
        char *random_id = generate_random_id(); // Attribute doesn't exist, generate and add it
        if (random_id == NULL) {
            fprintf(stderr, "Failed to generate random ID\n");
            return 1;
        }
        
        if (setxattr(filename, attr_name, random_id, strlen(random_id), 0) != 0) {
            perror("Failed to set extended attribute");
            free(random_id);
            return 1;
        }
        
        printf("Added ID %s to file %s\n", random_id, filename);
        free(random_id);
        return 0;
    }
}