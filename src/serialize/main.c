#include <stdio.h>
#include <stdlib.h>

extern unsigned char* serialize_header(); 

int main() {
    
    FILE *file = fopen("test.dat", "wb");
    
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }
   
    const unsigned char *data = serialize_header();
    size_t bytes_written = fwrite(data, 1, 32, file);
    
    if (bytes_written < 13) {
        fprintf(stderr, "Error writing to file\n");
    } else {
        printf("Successfully wrote %zu bytes to file\n", bytes_written);
    }
    
  
    fclose(file);
    
    return 0;
}
