#include <stdio.h>   // For fprintf, printf, stderr
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For strncpy

inline struct nested_node* create_nested_node(const char* name) {
    struct nested_node* new_node = (struct nested_node*)malloc(sizeof(struct nested_node));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    strncpy(new_node->name, name, MAX_NAME_CHAR - 1);
    new_node->name[MAX_NAME_CHAR - 1] = '\0'; 
    new_node->parent = NULL;
    new_node->child_count = 0;
    

    for (int i = 0; i < MAX_CHILDREN; i++) {
        new_node->children[i] = NULL;
    }
    
    return new_node;
}

inline int add_nested_child(struct nested_node* parent, struct nested_node* child) {
    if (parent->child_count >= MAX_CHILDREN) {
        return 0; 
    }
    
    parent->children[parent->child_count] = child;
    child->parent = parent;
    parent->child_count++;
    return 1;
}


