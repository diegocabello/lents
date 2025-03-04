#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tree_from_yaml.h"

#define MAX_NAME_CHAR 56
#define MAX_CHILDREN 16

struct node {
    char uid[3];
    char hash[5];
    char name[MAX_NAME_CHAR];
    struct node *parent;         
    struct node *children[MAX_CHILDREN]; 
    int child_count;
};

struct node* create_node(const char* name) {
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
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

int add_child(struct node* parent, struct node* child) {
    if (parent->child_count >= MAX_CHILDREN) {
        return 0; // Cannot add more children
    }
    
    parent->children[parent->child_count] = child;
    child->parent = parent;
    parent->child_count++;
    return 1;
}

void print_tree(struct node *root, int depth) {
    if (!root) return;

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    printf("- %s\n", root->name);

    for (int i = 0; i < root->child_count; i++) {
        print_tree(root->children[i], depth + 1);
    }
}

void free_tree(struct node *root) {
    if (root == NULL) {
        return;
    }

    // First free all children recursively
    for (int i = 0; i < root->child_count; i++) {
        free_tree(root->children[i]);
    }

    // Then free the node itself
    free(root);
}

int main() {
    // Load the tree from YAML
    struct node *tree = build_tree_from_yaml("categories.yaml");
    
    if (!tree) {
        fprintf(stderr, "Failed to build tree from YAML\n");
        return 1;
    }
    
    // Print the tree for verification
    printf("Category Tree:\n");
    print_tree(tree, 0);
    
    // Free all memory used by the tree
    free_tree(tree);
    
    return 0;
}
