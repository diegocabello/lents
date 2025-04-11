#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gents.h"

extern int serialize(const char* input_filename, const char* output_filename);

struct node* create_node(const char* name);
void free_tree(struct node* root);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s parse <input_file> [output_file]\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "parse") == 0) {
        const char* input_file = argv[2];
        const char* output_file = "lents.nts"; // Default output filename
        
        if (argc > 3) {
            output_file = argv[3];
        }
        
        if (serialize(input_file, output_file)) {
            printf("File serialized successfully to %s\n", output_file);
        } else {
            fprintf(stderr, "Failed to serialize file\n");
            return 1;
        }
    } else if (strcmp(argv[1], "tag") == 0) {

    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }
    
    return 0;
}
