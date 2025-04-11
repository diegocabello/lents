#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gents.h"

extern int serialize(const char* input_filename, const char* output_filename);

struct node* create_node(const char* name);
void free_tree(struct node* root);

// MUST BE NULL TERMINATED
const char *TAG_TO_FILES_ALIASES[] = {"tagtofiles", "ttf", NULL};
const char *FILE_TO_TAGS_ALIASES[] = {"filetotags", "ftt", NULL};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s parse <input_file> [output_file]\n", argv[0]);
        return 1;
    }
    
    const char* command = argv[1];

    if (strcmp(command, "parse") == 0) {
        const char* input_file = argv[2];
        const char* output_file = "lents.nts"; // Default output filename
        
        if (argc > 3) {
            output_file = argv[3];
        }
        
        if (serialize(input_file, output_file)) {
            printf("Data file serialized successfully to %s\n", output_file);
        } else {
            fprintf(stderr, "Failed to serialize data file\n");
            return 1;
        }
        
    } else if (is_alias(command, FILE_TO_TAGS_ALIASES)) {
        
        const char* filename = argv[2];
        const char* operation = argv[3]; // add or remove
        const char* attr_name = "user.lents_id";  
        
        int result = file_to_tags(filename, attr_name, operation);
        return result ? 0 : 1;  

    } else if (is_alias(command, TAG_TO_FILES_ALIASES)) {
        // functionality
    } else if (strcmp(command, "filter") == 0) {
        // functionality
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }
    
    return 0;
}