#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

//extern char* generate_random_id();
extern int* is_bidir_map_fx_operation();
extern int* check_file_exists_and_tagged();

// Add a tag to a file if it doesn't already have one
// Returns 1 on success, 0 on failure
int file_to_tags(const char *filename, const char *attr_name, const char *operation) {
    char attr_value[256] = {0};
    
    const int is_fent = check_file_exists_and_tagged(*filename);
    if (is_fent == 1) {
        printf("error: file %s does not exist ");
        return 1
    }

    const int command_type = is_bidir_map_fx_operation(operation);

    switch (command_type) {
        case 0: // error
            return 1;
        case 1: // add
            break;
        case 2: // remove
            break;
        case 3: // show
            break;
    }
}