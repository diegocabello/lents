#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gents.h"

// Create a new node with the given name
struct nested_node* create_nested_node(const char* name) {
    struct nested_node* new_node = (struct nested_node*)malloc(sizeof(struct nested_node));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    strncpy(new_node->name, name, MAX_NAME_CHAR - 1);
    new_node->name[MAX_NAME_CHAR - 1] = '\0'; // Ensure null termination
    new_node->parent = NULL;
    new_node->child_count = 0;
    
    // Initialize all child pointers to NULL
    for (int i = 0; i < MAX_CHILDREN; i++) {
        new_node->children[i] = NULL;
    }
    
    return new_node;
}

// Add a child to a parent node
int add_nested_child(struct nested_node* parent, struct nested_node* child) {
    if (parent->child_count >= MAX_CHILDREN) {
        return 0; // Cannot add more children
    }
    
    parent->children[parent->child_count] = child;
    child->parent = parent;
    parent->child_count++;
    return 1;
}

// Helper function to print the tree (for debugging)
void print_nested_tree(struct nested_node *root, int depth) {
    if (!root) return;

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    printf("- %s\n", root->name);

    for (int i = 0; i < root->child_count; i++) {
        print_nested_tree(root->children[i], depth + 1);
    }
}

// Helper function to free the tree
void free_nested_tree(struct nested_node *root) {
    if (root == NULL) {
        return;
    }

    // First free all children recursively
    for (int i = 0; i < root->child_count; i++) {
        free_nested_tree(root->children[i]);
    }

    // Then free the node itself
    free(root);
}

// Serialize the nested tree structure
unsigned char* serialize_nested(struct nested_node* root) {
    // This function will be implemented to serialize the tree structure
    // For now, return NULL as a placeholder
    return NULL;
}

/**
 * Serializes a single tree to a binary file
 * 
 * @param file The output file
 * @param node The root node of the tree to serialize
 * @return 1 on success, 0 on failure
 */
int serialize_tree(FILE* file, struct nested_node* node) {
    if (!file || !node) return 0;
    
    // Write node name length
    uint8_t name_len = strlen(node->name);
    if (fwrite(&name_len, 1, 1, file) != 1) {
        perror("Error writing name length");
        return 0;
    }
    
    // Write node name
    if (fwrite(node->name, 1, name_len, file) != name_len) {
        perror("Error writing name");
        return 0;
    }
    
    // Write number of children
    if (fwrite(&node->child_count, 1, 1, file) != 1) {
        perror("Error writing child count");
        return 0;
    }
    
    // Recursively serialize each child
    for (int i = 0; i < node->child_count; i++) {
        if (!serialize_tree(file, node->children[i])) {
            return 0;
        }
    }
    
    return 1;
}