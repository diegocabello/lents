#ifndef GENTS_H
#define GENTS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC_NUMBER "GENTS"
#define VERSION_NUMBER 1

#define MAX_FOREST_SIZE 23
#define MAX_CHILDREN 15
#define MAX_DEPTH 8
#define MAX_NAME_CHAR 44 
#define HASH_BYTE_LENGTH 7

struct serialized_child_node{
    uint32_t uid;
    char has_children;
};

struct serialized_node{
    char hash[7];
    uint32_t uid;
    char name[MAX_NAME_CHAR];
    uint32_t ancestry_uids[MAX_DEPTH];
    struct serialized_child_node children[MAX_CHILDREN];
};

struct memory_node {
    uint32_t uid;
    uint32_t ancestry_uids[8];
    char name[52];
    struct memory_node *children[15];
};

// Define the nested_node structure
struct nested_node {
    char name[MAX_NAME_CHAR];
    struct nested_node* parent;
    struct nested_node* children[MAX_CHILDREN];
    int child_count;
    uint32_t uid;
};

// Create a new node with the given name
static inline struct nested_node* create_nested_node(const char* name) {
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
static inline int add_nested_child(struct nested_node* parent, struct nested_node* child) {
    if (parent->child_count >= MAX_CHILDREN) {
        return 0; // Cannot add more children
    }
    
    parent->children[parent->child_count] = child;
    child->parent = parent;
    parent->child_count++;
    return 1;
}

// Helper function to print the tree (for debugging)
static inline void print_nested_tree(struct nested_node *root, int depth) {
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
static inline void free_nested_tree(struct nested_node *root) {
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

// Forward declaration for YAML parser
struct nested_node* build_tree_from_yaml(const char* filename);

#endif /* GENTS_H */
