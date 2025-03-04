#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SIZE_UNIT 16 
#define MAX_SIZE 4
#define DATAFILE "tiftsuid.dat"

void to_lowercase(char *s) {
    for (int i = 0; s[i]; i++) s[i] = tolower((unsigned char)s[i]);
}

void print_uid_binary(unsigned char uid) {
    for (int i = 7; i >= 0; i--) {
        putchar((uid & (1 << i)) ? '1' : '0');
    }
    putchar('\n');
}

int add_tag(const char *tag) {
    FILE *fp = fopen(DATAFILE, "r+b");
    if (!fp) {
        fp = fopen(DATAFILE, "w+b");
        if (!fp) return 1;
    }
    char lower_tag[128];
    memset(lower_tag, 0, sizeof(lower_tag));
    strncpy(lower_tag, tag, sizeof(lower_tag)-1);
    to_lowercase(lower_tag);
    size_t len = strlen(lower_tag);
    int rec_size;
    int name_len;
    
    //check if it will fit
    if (len > (SIZE_UNIT * MAX_SIZE - 2)) {
        fprintf(stderr, "error: tag name cannot exceed %d bytes\n", (SIZE_UNIT * MAX_SIZE - 2));
        fclose(fp);
        return 1;
    }
    
    // Find the smallest suitable size bucket
    for (int i = 1; i <= MAX_SIZE; i++) {        
        if (len <= (SIZE_UNIT * i - 2)) { 
            rec_size = SIZE_UNIT * i; 
            name_len = rec_size - 2;
            break;
        }
    } 
    
    //check if the tag is already in there, and count the uuids up 

    unsigned char record[128];
    memset(record, 0, rec_size);
    unsigned char highest_uid = 0;
    rewind(fp);

    while (1) {
        // Read first byte (size indicator) and calculate record size based on the first byte
        if (fread(record, 1, 1, fp) != 1) break;
        int current_size = SIZE_UNIT * (record[0] + 1);
        
        // Read the rest of the record (including UID and tag) and update highest UID seen
        if (fread(record+1, 1, current_size-1, fp) != current_size-1) break;
        if (record[1] > highest_uid) {
            highest_uid = record[1];
        }
        
        // Check if tag already exists
        if (strncmp((char*)record+2, lower_tag, current_size-2) == 0) {
            printf("Tag \"%s\" already exists with UID: ", tag);
            print_uid_binary(record[1]);
            fclose(fp);
            return 0;
        }
    }
    
    unsigned char uid = highest_uid + 1;
    memset(record, 0, rec_size);
    record[0] = (rec_size / SIZE_UNIT) - 1;  // 0 for 16, 1 for 32, 2 for 48, 3 for 64
    record[1] = uid;
    strncpy((char*)record+2, lower_tag, name_len);
    fseek(fp, 0, SEEK_END);
    if (fwrite(record, 1, rec_size, fp) != rec_size) {
        fclose(fp);
        return 1;
    }
    
    fclose(fp);
    printf("added tag %d byte entry \"%s\" with UID: ", rec_size, tag);
    print_uid_binary(uid);
    return 0;
}

int index_tag(const char *tag) {
    FILE *fp = fopen(DATAFILE, "rb");
    if (!fp) { 
        printf("Tag \"%s\" is not in the table.\n", tag); 
        return 1;
    }
    
    char lower_tag[128];
    memset(lower_tag, 0, sizeof(lower_tag));
    strncpy(lower_tag, tag, sizeof(lower_tag)-1);
    to_lowercase(lower_tag);
    
    unsigned char record[128];
    
    while (1) {
        // Read the first byte (size indicator)
        if (fread(record, 1, 1, fp) != 1) break;
        
        // Determine record size based on first byte (0-3)
        int size_multiple = record[0] + 1; // Convert to 1-4
        int rec_size = SIZE_UNIT * size_multiple;
        
        // Read the UID and tag name
        if (fread(record+1, 1, rec_size-1, fp) != rec_size-1) break;
        
        // Check if tag matches
        int name_len = rec_size - 2;
        if (strncmp((char*)record+2, lower_tag, name_len) == 0) {
            printf("Tag \"%s\" found with UID: ", tag);
            print_uid_binary(record[1]);
            fclose(fp);
            return 0;
        }
    }
    
    fclose(fp);
    printf("Tag \"%s\" is not in the table.\n", tag);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s add|index \"tag string\"\n", argv[0]);
        return 1;
    }
    const char *command = argv[1];
    const char *tag = argv[2];
    if (strcmp(command, "add") == 0) {
        return add_tag(tag);
    } else if (strcmp(command, "index") == 0) {
        return index_tag(tag);
    } else {
        printf("Unknown command: %s\n", command);
        return 1;
    }
}

