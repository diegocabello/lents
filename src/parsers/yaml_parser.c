// tree_from_yaml.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>
#include "tree_from_yaml.h" // We'll create this header file next

// Forward declarations of helper functions
static struct node* process_yaml_mapping(yaml_parser_t *parser, struct node *parent);
static void process_yaml_sequence(yaml_parser_t *parser, struct node *parent);

struct node* build_tree_from_yaml(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }
    
    yaml_parser_t parser;
    yaml_event_t event;
    
    // Initialize parser
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize YAML parser\n");
        fclose(file);
        return NULL;
    }
    
    yaml_parser_set_input_file(&parser, file);
    
    // Root node - we'll determine its name from the YAML
    struct node* root = NULL;
    
    // Parse events
    do {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "Parser error: %d\n", parser.error);
            yaml_parser_delete(&parser);
            fclose(file);
            return NULL;
        }
        
        // We're only interested in document start event to begin processing
        if (event.type == YAML_DOCUMENT_START_EVENT) {
            // Start processing from the root
            root = process_yaml_mapping(&parser, NULL);
            break;
        }
        
        yaml_event_delete(&event);
    } while (event.type != YAML_STREAM_END_EVENT);
    
    // Cleanup
    yaml_parser_delete(&parser);
    fclose(file);
    
    return root;
}

// Process a YAML mapping (key-value pairs)
static struct node* process_yaml_mapping(yaml_parser_t *parser, struct node *parent) {
    yaml_event_t event;
    struct node *current_node = NULL;
    char *key = NULL;
    
    while (1) {
        if (!yaml_parser_parse(parser, &event)) {
            fprintf(stderr, "Parser error in mapping\n");
            return NULL;
        }
        
        // End of the mapping
        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }
        
        // Get the key (category name)
        if (event.type == YAML_SCALAR_EVENT) {
            if (key == NULL) {
                // This is a key
                key = strdup((char *)event.data.scalar.value);
                
                // Create a node for this key
                current_node = create_node(key);
                
                // Add to parent if needed
                if (parent != NULL) {
                    add_child(parent, current_node);
                }
                
                free(key);
                key = NULL;
            } else {
                // This is a scalar value (we don't expect this in a nested tree)
                fprintf(stderr, "Unexpected scalar value: %s\n", (char *)event.data.scalar.value);
                free(key);
                key = NULL;
            }
        }
        // Handle sequence (list of child categories)
        else if (event.type == YAML_SEQUENCE_START_EVENT && key != NULL) {
            process_yaml_sequence(parser, current_node);
            free(key);
            key = NULL;
        }
        // Handle nested mapping
        else if (event.type == YAML_MAPPING_START_EVENT && key != NULL) {
            process_yaml_mapping(parser, current_node);
            free(key);
            key = NULL;
        }
        
        yaml_event_delete(&event);
    }
    
    return current_node;
}

// Process a YAML sequence (list of items)
static void process_yaml_sequence(yaml_parser_t *parser, struct node *parent) {
    yaml_event_t event;
    
    while (1) {
        if (!yaml_parser_parse(parser, &event)) {
            fprintf(stderr, "Parser error in sequence\n");
            return;
        }
        
        // End of the sequence
        if (event.type == YAML_SEQUENCE_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }
        
        // Handle a scalar item (simple child category)
        if (event.type == YAML_SCALAR_EVENT) {
            struct node *child = create_node((char *)event.data.scalar.value);
            add_child(parent, child);
        }
        // Handle a mapping item (child category with its own children)
        else if (event.type == YAML_MAPPING_START_EVENT) {
            process_yaml_mapping(parser, parent);
        }
        
        yaml_event_delete(&event);
    }
}
