#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gents.h"

extern unsigned char* serialize_header();
extern struct nested_node* build_tree_from_yaml(const char* filename);
extern int serialize_tree(FILE* file, struct nested_node* node);

/**
 * Get file extension from filename
 * 
 * @param filename The input filename
 * @return Pointer to the extension part of the filename, or NULL if no extension
 */

const char* get_file_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return NULL;
    }
    return dot + 1;
}

/**
 * Serializes the entire tree structure to a binary file
 * 
 * @param input_filename The input filename (YAML, JSON, etc.)
 * @param output_filename The output binary filename
 * @return 1 on success, 0 on failure
 */
int serialize(const char* input_filename, const char* output_filename) {
    if (!input_filename || !output_filename) return 0;
    
    // Determine file type based on extension
    const char* extension = get_file_extension(input_filename);
    if (!extension) {
        fprintf(stderr, "Error: Input file has no extension\n");
        return 0;
    }
    
    // Build tree based on file type
    struct nested_node* root = NULL;
    
    if (strcasecmp(extension, "yaml") == 0 || strcasecmp(extension, "yml") == 0) {
        root = build_tree_from_yaml(input_filename);
    } else {
        fprintf(stderr, "Error: Unsupported file format: %s\n", extension);
        return 0;
    }
    
    if (!root) {
        fprintf(stderr, "Error: Failed to build tree from input file\n");
        return 0;
    }
    
    // Open output file
    FILE* file = fopen(output_filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", output_filename);
        free_nested_tree(root);
        return 0;
    }
    
    // Write header
    unsigned char* header = serialize_header();
    if (!header) {
        fprintf(stderr, "Error: Failed to serialize header\n");
        fclose(file);
        free_nested_tree(root);
        return 0;
    }
    
    // Write the header to the file
    if (fwrite(header, 1, 16, file) != 16) {
        fprintf(stderr, "Error: Failed to write header to file\n");
        free(header);
        fclose(file);
        free_nested_tree(root);
        return 0;
    }
    
    // For a YAML file, there's typically just one root node
    int forest_size = 1;

    // Just serialize the root node - it will recursively handle all children
    if (!serialize_tree(file, root)) {
        fprintf(stderr, "Error: Failed to serialize tree\n");
        free(header);
        fclose(file);
        free_nested_tree(root);
        return 0;
    }
    
    // Cleanup
    free(header);
    free_nested_tree(root);
    fclose(file);
    return 1;
}
