// tree_from_yaml.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // Add this for isspace()
#include <yaml.h>
#include "gents.h"

// Forward declarations of helper functions
static struct nested_node* process_yaml_mapping(yaml_parser_t *parser, struct nested_node *parent, int current_depth);
static void process_yaml_sequence(yaml_parser_t *parser, struct nested_node *parent);

// Build a tree from a YAML file
struct nested_node* build_tree_from_yaml(const char* filename) {
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
    
    // Create a root node to hold all top-level categories
    struct nested_node* forest_root = create_nested_node("ROOT");
    
    // Parse events with safety counter
    int event_count = 0;
    int max_events = 1000; // Safety limit
    int done = 0;
    
    while (!done && event_count < max_events) {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "Parser error: %d\n", parser.error);
            yaml_parser_delete(&parser);
            fclose(file);
            return NULL;
        }
        
        event_count++;
        printf("DEBUG: Event #%d, type: %d\n", event_count, event.type);
        
        if (event.type == YAML_NO_EVENT) {
            fprintf(stderr, "Warning: Received YAML_NO_EVENT, stopping parser\n");
            done = 1;
        }
        else if (event.type == YAML_STREAM_END_EVENT) {
            printf("DEBUG: End of YAML stream\n");
            done = 1;
        }
        else if (event.type == YAML_DOCUMENT_START_EVENT) {
            printf("DEBUG: Found document start\n");
            
            // Process the document content - look for a mapping or sequence start
            yaml_event_t content_event;
            if (yaml_parser_parse(&parser, &content_event)) {
                event_count++;
                printf("DEBUG: Document content event type: %d\n", content_event.type);
                
                if (content_event.type == YAML_MAPPING_START_EVENT) {
                    // Process mapping and its contents
                    yaml_event_delete(&content_event);
                    if (!process_yaml_mapping(&parser, forest_root, 0)) {
                        fprintf(stderr, "Error processing mapping\n");
                    }
                }
                else if (content_event.type == YAML_SEQUENCE_START_EVENT) {
                    // Process sequence and its contents
                    printf("DEBUG: Processing root sequence\n");
                    yaml_event_delete(&content_event);
                    process_yaml_sequence(&parser, forest_root);
                    printf("DEBUG: Finished processing root sequence\n");
                }
                else {
                    fprintf(stderr, "Unexpected document content type: %d\n", content_event.type);
                    yaml_event_delete(&content_event);
                }
            }
        }
        
        yaml_event_delete(&event);
    }
    
    if (event_count >= max_events) {
        fprintf(stderr, "Warning: Reached maximum event count, parser might be in a loop\n");
    }
    
    // Cleanup
    yaml_parser_delete(&parser);
    fclose(file);
    
    // Print the tree for debugging
    printf("Parsed YAML tree:\n");
    print_nested_tree(forest_root, 0);
    
    return forest_root;
}

// Process a YAML mapping (key-value pairs)
static struct nested_node* process_yaml_mapping(yaml_parser_t *parser, struct nested_node *parent, int current_depth) {
    // Check depth limit
    if (current_depth >= MAX_DEPTH) {
        fprintf(stderr, "Warning: A tree cannot have depth over %d. Deeper nodes ignored.\n", MAX_DEPTH);
        // Skip this mapping but don't crash
        yaml_event_t event;
        int mapping_level = 1;
        while (mapping_level > 0) {
            if (!yaml_parser_parse(parser, &event)) break;
            if (event.type == YAML_MAPPING_START_EVENT) mapping_level++;
            else if (event.type == YAML_MAPPING_END_EVENT) mapping_level--;
            yaml_event_delete(&event);
        }
        return NULL;
    }
    
    yaml_event_t event;
    struct nested_node *current_node = NULL;
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
                current_node = create_nested_node(key);
                
                // Add to parent if needed
                if (parent != NULL) {
                    add_nested_child(parent, current_node);
                }
                
                // Check for the next event to see if it's a value or another mapping/sequence
                yaml_event_t next_event;
                if (yaml_parser_parse(parser, &next_event)) {
                    // If it's a scalar, it's a key-value pair (error)
                    if (next_event.type == YAML_SCALAR_EVENT) {
                        // Check if it's an empty value (just whitespace)
                        char* value = (char *)next_event.data.scalar.value;
                        int is_empty = 1;
                        for (int i = 0; value[i]; i++) {
                            if (!isspace(value[i])) {
                                is_empty = 0;
                                break;
                            }
                        }
                        
                        if (is_empty) {
                            // This is fine - it's an empty node
                            yaml_event_delete(&next_event);
                        } else {
                            // This is a real key-value pair, which we don't want
                            fprintf(stderr, "Error: GENTS does not accept key-value pairs\n");
                            fprintf(stderr, "Found value '%s' for key\n", value);
                            yaml_event_delete(&next_event);
                            free(key);
                            return NULL;
                        }
                    }
                    // If it's a sequence, process it
                    else if (next_event.type == YAML_SEQUENCE_START_EVENT) {
                        process_yaml_sequence(parser, current_node);
                    }
                    // If it's a mapping, process it
                    else if (next_event.type == YAML_MAPPING_START_EVENT) {
                        process_yaml_mapping(parser, current_node, current_depth + 1);
                    }
                    // If it's the end of the mapping, this key has no value (which is fine)
                    else if (next_event.type == YAML_MAPPING_END_EVENT) {
                        // Just delete the event and continue processing other keys
                        yaml_event_delete(&next_event);
                        // Don't break here - we need to continue processing other keys
                    }
                    
                    yaml_event_delete(&next_event);
                }
                
                free(key);
                key = NULL;
            }
        }
        
        yaml_event_delete(&event);
    }
    
    return parent;
}

// Process a YAML sequence (list of items)
static void process_yaml_sequence(yaml_parser_t *parser, struct nested_node *parent) {
    yaml_event_t event;
    int item_count = 0;
    int max_items = 1000; // Safety limit
    
    while (item_count < max_items) {
        if (!yaml_parser_parse(parser, &event)) {
            fprintf(stderr, "Parser error in sequence\n");
            return;
        }
        
        printf("DEBUG: Sequence event type: %d\n", event.type);
        
        // End of the sequence
        if (event.type == YAML_SEQUENCE_END_EVENT) {
            printf("DEBUG: End of sequence after %d items\n", item_count);
            yaml_event_delete(&event);
            break;
        }
        
        // Handle a scalar item (simple child category)
        if (event.type == YAML_SCALAR_EVENT) {
            item_count++;
            printf("DEBUG: Processing sequence item #%d: %s\n", item_count, (char *)event.data.scalar.value);
            struct nested_node *child = create_nested_node((char *)event.data.scalar.value);
            add_nested_child(parent, child);
        }
        // Handle a mapping item (child category with its own children)
        else if (event.type == YAML_MAPPING_START_EVENT) {
            item_count++;
            printf("DEBUG: Processing mapping in sequence, item #%d\n", item_count);
            
            // Create a temporary node to hold mapping data
            struct nested_node *temp_node = create_nested_node("item");
            add_nested_child(parent, temp_node);
            
            // Process the mapping with the correct depth
            process_yaml_mapping(parser, temp_node, 1);
        }
        
        yaml_event_delete(&event);
    }
    
    if (item_count >= max_items) {
        fprintf(stderr, "Warning: Reached maximum number of items in sequence\n");
    }
}
