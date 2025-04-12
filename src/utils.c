#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//!!THESE ALL HAVE TO BE TERMINATED WITH NULL ELSE THE PROGRAM WILL HAVE PROBLEMS!!
const char *REMOVE_ALIASES[] ={"remove", "rm", NULL};
const char *ASSIGN_ALIASES[] ={"assign", "add", NULL};

const unsigned char FILE_TAGS_STRUCTURE_MARKER[FILE_TAGS_STRUCTURE_MARKER_SIZE] = {
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
};

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
    if (is_alias(operation, ASSIGN_ALIASES)) {
        return 1;
    } else if (is_alias(operation, REMOVE_ALIASES)) {
        return 2;
    } else if (strcmp(operation, "show") == 0) {
        return 3;
    } else {
        return 0;
    }
}

char* check_file_exists_and_tagged(const char* filename) {
    // Check if file exists
    if (access(filename, F_OK) != 0) {
        fprintf(stderr, "File %s does not exist\n", filename);
        return NULL;
    }
    
    char attr_value[256] = {0};
    const char* attr_name;
    ssize_t attr_size;
    
    #ifdef __APPLE__
        // macOS/APFS implementation
        attr_name = "com.ents.uid";
        attr_size = getxattr(filename, attr_name, attr_value, sizeof(attr_value) - 1, 0, 0);
    #else
        // Linux/EXT4 implementation
        attr_name = "user.ents_uid";
        attr_size = getxattr(filename, attr_name, attr_value, sizeof(attr_value) - 1);
    #endif
    
    // If attribute exists, return a copy of it
    if (attr_size > 0) {
        attr_value[attr_size] = '\0';  // Ensure null termination
        return strdup(attr_value);     // Return a copy (caller must free)
    }
    
    // Attribute doesn't exist, generate and add it
    char* random_id = generate_random_id();
    if (random_id == NULL) {
        fprintf(stderr, "Failed to generate random ID\n");
        return NULL;
    }
    
    int result;
    #ifdef __APPLE__
        result = setxattr(filename, attr_name, random_id, strlen(random_id), 0, 0);
    #else
        result = setxattr(filename, attr_name, random_id, strlen(random_id), 0);
    #endif
    
    if (result != 0) {
        perror("Failed to set extended attribute");
        free(random_id);
        return NULL;
    }
    
    return random_id;  // Caller must free this memory
}

// might need to add some out of memory errors
unsigned char* check_file_in_db(unsigned char *data_pointer, unsigned char *file_id) {
    unsigned char buffer[96];
    
    while (1) {
        memcpy(buffer, data_pointer, 96); // Load 96 bytes from the data pointer
        
        if (memcmp(buffer, file_id, 7) == 0) { // Compare first 7 bytes with the file_id
            return data_pointer; // Return address where file_id was found
        }
        
        data_pointer += 96; // Move pointer to next record
        
        if (*data_pointer == '\0' && *(data_pointer+1) == '\0') {
            char *random_id = generate_random_id();
            memcpy(buffer, random_id, 7);
            memset(data_pointer + 7, 0, 89); // Fill the rest of the buffer with zeros
            
            free(random_id);

            return buffer;
        }
    }
}

int tag_file_common(unsigned char *fattr_memory, unsigned char *tag, bool is_remove) {
    unsigned char *current_pos = fattr_memory + 8;
    
    for (int i = 0; i < 11; i++) {
        if (memcmp(current_pos, tag, 7) == 0) {
            if (is_remove) {
                memset(current_pos, 0, 8); // Write zeroes if removing
            }
            return 0; // Success
        }
        
        int is_empty = 1; // Assume position is empty
        for (int j = 0; j < 8; j++) { // Check all 8 bytes
            if (current_pos[j] != 0) { // If any byte is non-zero
                is_empty = 0; // Position is not empty
                break;
            }
        }
        
        if (is_empty) { // If empty spot found
            memcpy(current_pos, tag, 7); // Copy tag to empty spot
            return 0; // Success
        }
        
        current_pos += 8; // Move to next tag position
    }
    
    printf("Error: Out of tag space for this file\n"); // All spots are full
    return 1; // Failure
}

int assign_tag_to_file(unsigned char *fattr_memory, unsigned char *tag) {
    return tag_file_common(fattr_memory, tag, false);
}

int remove_tag_from_file(unsigned char *fattr_memory, unsigned char *tag) {
    return tag_file_common(fattr_memory, tag, true);
}