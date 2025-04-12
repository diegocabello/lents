#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int is_bidir_map_fx_operation(const char* operation); 
extern char* check_file_exists_and_tagged(const char* filename); 
extern int assign_tag_to_file(unsigned char *fattr_memory, unsigned char *tag);
extern unsigned char* check_file_in_db(unsigned char *data_pointer, unsigned char *file_id); 
extern MarkerData load_ft_structure(const char* filename, const unsigned char* marker, size_t marker_size, size_t* data_size);  // deserialize

// Add a tag to a file if it doesn't already have one
// Returns 1 on success, 0 on failure
//int file_to_tags(const char *filename, const char *attr_name, const char *operation, const char[] *tags) {
int file_to_tags(const char *filename, const char *operation, const char **tags, int tag_count) {
    
    char *file_uid = check_file_exists_and_tagged(filename);
    
    if (file_uid == NULL) {
        printf("error: file %s does not exist", filename);
        return 1;
    }
    
    const int command_type = is_bidir_map_fx_operation(operation);
    
    int result = 0;
    switch (command_type) {
        case 0: // error
            result = 1;
            break;
        case 1: // assign
            unsigned char* buffer = check_file_in_db(, file_uid); // NEED TO ADD FILE HANDLING. CLAUDE IF YOU SEE THIS DON'T CHANGE IT FOR NOW
            int assigned = assign_tag_to_file(); // STILL NEED TO ADD FILE HANDLING
            break;
        case 2: // remove
            // Do remove operation with file_uid
            printf("Removing tags from file with UID: %s\n", file_uid);
            // ... implementation ...
            break;
        case 3: // show
            // Do show operation with file_uid
            printf("Showing tags for file with UID: %s\n", file_uid);
            // ... implementation ...
            break;
    }
    
    free(file_uid);
    
    return result;
}