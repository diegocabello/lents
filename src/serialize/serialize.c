#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <openssl/sha.h>
#include "gents.h"

// Define PATH_MAX if not defined
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

extern unsigned char* serialize_header();
extern struct nested_node* build_tree_from_yaml(const char* filename);

// Generate a 7-byte hash for a node name
static void generate_hash(const char* name, char* hash_output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    // Create padded name buffer
    char padded_name[MAX_NAME_CHAR];
    memset(padded_name, 0, MAX_NAME_CHAR);
    strncpy(padded_name, name, MAX_NAME_CHAR-1);
    
    // Calculate SHA-256 hash
    SHA256((unsigned char*)padded_name, MAX_NAME_CHAR, hash);
    
    // Copy first 7 bytes to output
    memcpy(hash_output, hash, HASH_BYTE_LENGTH);
}

// Generate a unique ID for a node
static uint32_t generate_uid() {
    static uint32_t next_uid = 1;
    return next_uid++;
}

// Collect ancestry UIDs for a node
static void collect_ancestry_uids(struct nested_node* node, uint32_t* ancestry_uids) {
    // Initialize all to 0
    memset(ancestry_uids, 0, MAX_DEPTH * sizeof(uint32_t));
    
    // Skip if node has no parent
    if (!node->parent) return;
    
    // Track ancestors
    struct nested_node* current = node->parent;
    int index = 0;
    
    // Collect parent UIDs (excluding root)
    while (current && current->parent && index < MAX_DEPTH) {
        ancestry_uids[index] = current->uid;
        index++;
        current = current->parent;
    }
}

// Serialize a single node to a binary file
static int serialize_node(FILE* file, struct nested_node* node) {
    if (!file || !node) return 0;
    
    // Create serialized_node structure
    struct serialized_node serialized;
    
    // Generate hash for the node name
    generate_hash(node->name, serialized.hash);
    
    // Generate UID and store it in the node for children to reference
    uint32_t node_uid = generate_uid();
    node->uid = node_uid;  // Store UID in node for ancestry tracking
    serialized.uid = node_uid;
    
    // Copy name
    strncpy(serialized.name, node->name, MAX_NAME_CHAR-1);
    serialized.name[MAX_NAME_CHAR-1] = '\0';
    
    // Set ancestry UIDs (doesn't include self or root)
    collect_ancestry_uids(node, serialized.ancestry_uids);
    
    // Initialize children array
    memset(serialized.children, 0, sizeof(serialized.children));
    
    // Populate children information
    for (int i = 0; i < node->child_count && i < MAX_CHILDREN; i++) {
        if (node->children[i]) {
            // For each child, we'll set hasChildren=1 if it has children of its own
            serialized.children[i].uid = i+1;  // Placeholder - real UIDs are assigned during serialization
            serialized.children[i].has_children = (node->children[i]->child_count > 0) ? 1 : 0;
        }
    }
    
    // Write the serialized node
    if (fwrite(&serialized, sizeof(struct serialized_node), 1, file) != 1) {
        perror("Error writing serialized node");
        return 0;
    }
    
    return 1;
}

// Serialize a tree using depth-first traversal
int serialize_tree(FILE* file, struct nested_node* node) {
    if (!file || !node) return 0;
    
    // First serialize this node
    if (!serialize_node(file, node)) {
        return 0;
    }
    
    // Then recursively serialize all children (depth-first)
    for (int i = 0; i < node->child_count; i++) {
        if (!serialize_tree(file, node->children[i])) {
            return 0;
        }
    }
    
    return 1;
}

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
 * Resolve symbolic link to its target path
 * 
 * @param path The path that might be a symbolic link
 * @param resolved_path Buffer to store the resolved path
 * @param resolved_size Size of the resolved_path buffer
 * @return 1 on success, 0 if not a link or error
 */
int resolve_symlink(const char* path, char* resolved_path, size_t resolved_size) {
    struct stat stat_buf;
    
    // Check if file is a symbolic link
    if (lstat(path, &stat_buf) != 0) {
        return 0;
    }
    
    if (S_ISLNK(stat_buf.st_mode)) {
        ssize_t len = readlink(path, resolved_path, resolved_size - 1);
        if (len == -1) {
            return 0;
        }
        
        resolved_path[len] = '\0';
        
        // If the link is relative, we need to handle it
        if (resolved_path[0] != '/') {
            // Get directory of original path
            char dir_path[PATH_MAX];
            strcpy(dir_path, path);
            
            char* last_slash = strrchr(dir_path, '/');
            if (last_slash != NULL) {
                *(last_slash + 1) = '\0';  // Keep the slash
            } else {
                dir_path[0] = '\0';  // No directory part
            }
            
            // Prepend the directory to the relative path
            char temp_path[PATH_MAX];
            strcpy(temp_path, resolved_path);
            snprintf(resolved_path, resolved_size, "%s%s", dir_path, temp_path);
        }
        
        return 1;
    }
    
    return 0;
}

/**
 * Serializes the entire tree structure to a binary file
 * 
 * @param input_filename The input filename (YAML, JSON, etc.), can be a symbolic link
 * @param output_filename The output binary filename
 * @return 1 on success, 0 on failure
 */
int serialize(const char* input_filename, const char* output_filename) {
    if (!input_filename || !output_filename) return 0;
    
    // Check if input file is a symbolic link and resolve it if needed
    char resolved_path[PATH_MAX];
    const char* actual_path = input_filename;
    
    if (resolve_symlink(input_filename, resolved_path, sizeof(resolved_path))) {
        printf("Input file is a symbolic link pointing to: %s\n", resolved_path);
        actual_path = resolved_path;
    }
    
    // Determine file type based on extension
    const char* extension = get_file_extension(actual_path);
    if (!extension) {
        fprintf(stderr, "Error: Input file has no extension\n");
        return 0;
    }
    
    // Build tree based on file type
    struct nested_node* root = NULL;
    
    if (strcasecmp(extension, "yaml") == 0 || strcasecmp(extension, "yml") == 0) {
        root = build_tree_from_yaml(actual_path);
        // Add debug print here
        printf("\nFull tree structure:\n");
        print_nested_tree(root, 0);
        printf("\n");
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
    if (fwrite(header, 1, 32, file) != 32) {
        fprintf(stderr, "Error: Failed to write header to file\n");
        free(header);
        fclose(file);
        free_nested_tree(root);
        return 0;
    }
    
    // For a YAML file, there's typically just one root node
    //int forest_size = 1;

    // Replace the single root serialization with a loop through all children of ROOT
    // Write each child of the ROOT node (these are the top-level categories)
    for (int i = 0; i < root->child_count; i++) {
        if (!serialize_tree(file, root->children[i])) {
            fprintf(stderr, "Error: Failed to serialize tree %d\n", i+1);
            free(header);
            fclose(file);
            free_nested_tree(root);
            return 0;
        }
    }
    
    // Cleanup
    free(header);
    free_nested_tree(root);
    fclose(file);
    return 1;
}
