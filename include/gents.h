#ifndef GENTS_H
#define GENTS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>  /* For extended attributes */
#include <unistd.h>     /* For file system access */

#define MAGIC_NUMBER "GENTS"
#define VERSION_NUMBER 1
#define MAX_FOREST_SIZE 23
#define MAX_CHILDREN 15
#define MAX_DEPTH 15
#define MAX_NAME_CHAR 60
#define HASH_BYTE_LENGTH 6

#define FILE_TAGS_STRUCTURE_MARKER_SIZE 32 // it's gotta be this way
extern const unsigned char FILE_TAGS_STRUCTURE_MARKER[FILE_TAGS_STRUCTURE_MARKER_SIZE];

struct serialized_node {
    char hash[HASH_BYTE_LENGTH]; 
    char flags[2];
    char name[MAX_NAME_CHAR];
    uint32_t ancestry_uids[MAX_DEPTH];
};

struct memory_node {
    uint32_t uid;
    uint32_t ancestry_uids[MAX_DEPTH];
    char name[MAX_NAME_CHAR + 8];  /* Slightly larger for flexibility */
    struct memory_node *children[MAX_CHILDREN];
};

struct nested_node {
    char name[MAX_NAME_CHAR];
    struct nested_node* parent;
    struct nested_node* children[MAX_CHILDREN];
    int child_count;
    uint32_t uid;
};

typedef struct {
    unsigned char* data;
    size_t size;
} MarkerData;

#endif 