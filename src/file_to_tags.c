#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

//!!THESE ALL HAVE TO BE TERMINATED WITH NULL ELSE THE PROGRAM WILL HAVE PROBLEMS!!
const char *REMOVE_ALIASES[] ={"remove", "rm", NULL};
const char *ADD_ALIASES[] = {"add", NULL};

// Add a tag to a file if it doesn't already have one
// Returns 1 on success, 0 on failure
int file_to_tags(const char *filename, const char *attr_name, const char *operation) {
    char attr_value[256] = {0};
    
    // Check if file exists
    if (access(filename, F_OK) != 0) {
        fprintf(stderr, "File %s does not exist\n", filename);
        return 0;
    }
    
    // Check if the extended attribute already exists
    ssize_t attr_size = getxattr(filename, attr_name, attr_value, sizeof(attr_value) - 1);
    
    if (attr_size == 0) {
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

    if (is_alias(operation, ADD_ALIASES)) {
        // add functionality
    } else if (is_alias(operation, REMOVE_ALIASES)) {
        // remove functionality
    } else {
        fprintf(stderr, "Unknown operation %s for command tag\n", operation);
        return 1;
    }
}