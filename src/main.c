#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "gents.h"

extern int serialize(const char* input_filename, const char* output_filename);
extern int file_to_tags(const char *filename, const char *operation, const char **tags, int tag_count);

struct node* create_node(const char* name);
void free_tree(struct node* root);

// MUST BE NULL TERMINATED
const char *TAG_TO_FILES_ALIASES[] = {"tagtofiles", "ttf", NULL};
const char *FILE_TO_TAGS_ALIASES[] = {"filetotags", "ftt", NULL};
const char *PROCESS_ALIASES[] = {"process", "parse", NULL};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s parse <input_file>\n", argv[0]);
        return 1;
    }
    
    const char* command = argv[1];

    if (is_alias(command, PROCESS_ALIASES)) {
        const char* input_file = argv[2];
        
        if (serialize(input_file, ".lents")) {
            printf("Data file serialized successfully to ./.lents");
        } else {
            fprintf(stderr, "Failed to serialize data file\n");
            return 1;
        }
        
    } else if (is_alias(command, FILE_TO_TAGS_ALIASES)) {
        if (argc < 4) { fprintf(stderr, "Usage: %s filetotags <filename> <operation> [tag1] [tag2] ...\n", argv[0]); return 1; }
        
        const char* filename = argv[2];
        const char* operation = argv[3]; // add, remove, show
        
        const char** tags = NULL;
        int tag_count = 0;
        
        if (argc > 4) {
            tag_count = argc - 4;
            tags = (const char**)malloc(tag_count * sizeof(const char*));
            if (tags == NULL) { fprintf(stderr, "Memory allocation error\n"); return 1; }
            
            for (int i = 0; i < tag_count; i++) { tags[i] = argv[i + 4]; }
        }
        
        int result = file_to_tags(filename, operation, tags, tag_count);
        free(tags);
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
