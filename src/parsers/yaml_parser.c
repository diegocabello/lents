#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <yaml.h>
#include "gents.h"

static struct nested_node* process_yaml_mapping(yaml_parser_t *parser, struct nested_node *parent, int current_depth);
static void process_yaml_sequence(yaml_parser_t *parser, struct nested_node *parent);

extern inline struct nested_node* create_nested_node(const char* name);
extern inline int add_nested_child(struct nested_node* parent, struct nested_node* child);

struct nested_node* build_tree_from_yaml(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }
    
    yaml_parser_t parser;
    yaml_event_t event;
    
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize YAML parser\n");
        fclose(file);
        return NULL;
    }
    
    yaml_parser_set_input_file(&parser, file);
    
    struct nested_node* forest_root = create_nested_node("ROOT");
    
    int event_count = 0;
    int max_events = 1000;
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
            
            yaml_event_t content_event;
            if (yaml_parser_parse(&parser, &content_event)) {
                event_count++;
                printf("DEBUG: Document content event type: %d\n", content_event.type);
                
                if (content_event.type == YAML_MAPPING_START_EVENT) {
                    yaml_event_delete(&content_event);
                    if (!process_yaml_mapping(&parser, forest_root, 0)) {
                        fprintf(stderr, "Error processing mapping\n");
                    }
                }
                else if (content_event.type == YAML_SEQUENCE_START_EVENT) {
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
    
    yaml_parser_delete(&parser);
    fclose(file);
    
    return forest_root;
}

static struct nested_node* process_yaml_mapping(yaml_parser_t *parser, struct nested_node *parent, int current_depth) {
    if (current_depth >= MAX_DEPTH) {
        fprintf(stderr, "Warning: A tree cannot have depth over %d. Deeper nodes ignored.\n", MAX_DEPTH);
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
        
        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }
        
        if (event.type == YAML_SCALAR_EVENT) {
            if (key == NULL) {
                key = strdup((char *)event.data.scalar.value);
                
                current_node = create_nested_node(key);
                
                if (parent != NULL) {
                    add_nested_child(parent, current_node);
                }
                
                yaml_event_t next_event;
                if (yaml_parser_parse(parser, &next_event)) {
                    if (next_event.type == YAML_SCALAR_EVENT) {
                        char* value = (char *)next_event.data.scalar.value;
                        int is_empty = 1;
                        for (int i = 0; value[i]; i++) {
                            if (!isspace(value[i])) {
                                is_empty = 0;
                                break;
                            }
                        }
                        
                        if (is_empty) {
                            yaml_event_delete(&next_event);
                        } else {
                            fprintf(stderr, "Error: GENTS does not accept key-value pairs\n");
                            fprintf(stderr, "Found value '%s' for key\n", value);
                            yaml_event_delete(&next_event);
                            free(key);
                            return NULL;
                        }
                    }
                    else if (next_event.type == YAML_SEQUENCE_START_EVENT) {
                        process_yaml_sequence(parser, current_node);
                    }
                    else if (next_event.type == YAML_MAPPING_START_EVENT) {
                        process_yaml_mapping(parser, current_node, current_depth + 1);
                    }
                    else if (next_event.type == YAML_MAPPING_END_EVENT) {
                        yaml_event_delete(&next_event);
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

static void process_yaml_sequence(yaml_parser_t *parser, struct nested_node *parent) {
    yaml_event_t event;
    int item_count = 0;
    int max_items = 1000;
    
    while (item_count < max_items) {
        if (!yaml_parser_parse(parser, &event)) {
            fprintf(stderr, "Parser error in sequence\n");
            return;
        }
        
        printf("DEBUG: Sequence event type: %d\n", event.type);
        
        if (event.type == YAML_SEQUENCE_END_EVENT) {
            printf("DEBUG: End of sequence after %d items\n", item_count);
            yaml_event_delete(&event);
            break;
        }
        
        if (event.type == YAML_SCALAR_EVENT) {
            item_count++;
            printf("DEBUG: Processing sequence item #%d: %s\n", item_count, (char *)event.data.scalar.value);
            struct nested_node *child = create_nested_node((char *)event.data.scalar.value);
            add_nested_child(parent, child);
        }
        else if (event.type == YAML_MAPPING_START_EVENT) {
            item_count++;
            printf("DEBUG: Processing mapping in sequence, item #%d\n", item_count);
            
            struct nested_node *temp_node = create_nested_node("item");
            add_nested_child(parent, temp_node);
            
            process_yaml_mapping(parser, temp_node, 1);
        }
        
        yaml_event_delete(&event);
    }
    
    if (item_count >= max_items) {
        fprintf(stderr, "Warning: Reached maximum number of items in sequence\n");
    }
}
