#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <openssl/sha.h>
#include "gents.h"

extern unsigned char* serialize_header();
extern struct nested_node* build_tree_from_yaml(const char* filename);

static inline void free_nested_tree(struct nested_node *root) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < root->child_count; i++) {
        free_nested_tree(root->children[i]);
    }
    free(root);
}

static unsigned char* generate_hash(const char* name, char* hash_output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    char padded_name[MAX_NAME_CHAR+1];
    memset(padded_name, 0, MAX_NAME_CHAR+1);
    strncpy(padded_name, name, MAX_NAME_CHAR);
    
    return SHA256((unsigned char*)padded_name, MAX_NAME_CHAR, hash);   
}

static void collect_ancestry_uids(struct nested_node* node, uint32_t* ancestry_uids) {
    memset(ancestry_uids, 0, MAX_DEPTH * sizeof(uint32_t));
    
    if (!node->parent) return;
    
    struct nested_node* current = node->parent;
    int index = 0;
    
    while (current && current->parent && index < MAX_DEPTH) {
        ancestry_uids[index] = current->uid;
        index++;
        current = current->parent;
    }
}

static int serialize_node(FILE* file, struct nested_node* node) {
    if (!file || !node) return 0;
    
    struct serialized_node serialized;
    
    generate_hash(node->name, serialized.hash);

    memcpy(&node->uid, serialized.hash, sizeof(uint32_t));
    serialized.uid = node->uid;
    
    strncpy(serialized.name, node->name, MAX_NAME_CHAR-1);
    serialized.name[MAX_NAME_CHAR-1] = '\0';
    
    collect_ancestry_uids(node, serialized.ancestry_uids);
    
    memset(serialized.children, 0, sizeof(serialized.children));
    
    for (int i = 0; i < node->child_count && i < MAX_CHILDREN; i++) {
        if (node->children[i]) {
            serialized.children[i].uid = i+1;
            serialized.children[i].has_children = (node->children[i]->child_count > 0) ? 1 : 0;
        }
    }
    
    if (fwrite(&serialized, sizeof(struct serialized_node), 1, file) != 1) {
        perror("Error writing serialized node");
        return 0;
    }
    
    return 1;
}

int serialize_tree(FILE* file, struct nested_node* node) {
    if (!file || !node) return 0;
    
    if (!serialize_node(file, node)) {
        return 0;
    }
    
    for (int i = 0; i < node->child_count; i++) {
        if (!serialize_tree(file, node->children[i])) {
            return 0;
        }
    }
    
    return 1;
}

int serialize(const char* input_filename, const char* output_filename) {
    if (!input_filename || !output_filename) return 0;
    
    const char* actual_path = input_filename;
    const char* extension = strrchr(actual_path, '.');

    if (!extension || extension == actual_path) {
        fprintf(stderr, "Error: Input file has no extension\n");
        return 0;
    } else {
        extension += 1;
    }
    
    struct nested_node* root = NULL;
    
    if (strcasecmp(extension, "yaml") == 0 || strcasecmp(extension, "yml") == 0) {
        root = build_tree_from_yaml(actual_path);
        printf("\nFull tree structure:\n");
        printf("\n");
    } else {
        fprintf(stderr, "Error: Unsupported file format: %s\n", extension);
        return 0;
    }
    
    FILE* file = fopen(output_filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", output_filename);
        free_nested_tree(root);
        return 0;
    }
    
    unsigned char* header = serialize_header();

    if (!header) {
        fprintf(stderr, "Error: Failed to serialize header\n");
        fclose(file);
        free_nested_tree(root);
        return 0;
    }
    
    if (fwrite(header, 1, 32, file) != 32) {
        fprintf(stderr, "Error: Failed to write header to file\n");
        free(header);
        fclose(file);
        free_nested_tree(root);
        return 0;
    }
    
    for (int i = 0; i < root->child_count; i++) {
        if (!serialize_tree(file, root->children[i])) {
            fprintf(stderr, "Error: Failed to serialize tree %d\n", i+1);
            free(header);
            fclose(file);
            free_nested_tree(root);
            return 0;
        }
    }
    
    free(header);
    free_nested_tree(root);
    fclose(file);
    return 1;
}
