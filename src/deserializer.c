#include "gents.h"

extern const unsigned char FILE_TAGS_STRUCTURE_MARKER[FILE_TAGS_STRUCTURE_MARKER_SIZE];

MarkerData load_after_marker(const char* filename, const unsigned char* marker, size_t marker_size) {
    MarkerData result = {NULL, 0};
    
    FILE* file = fopen(filename, "rb");
    if (!file) { perror("Failed to open file"); return result; }
    
    fseek(file, 0, SEEK_END); // Get file size
    long file_size = ftell(file);
    rewind(file);
    
    unsigned char* check_buffer = malloc(marker_size);
    if (!check_buffer) { fclose(file); return result; }
    
    long position = 0;
    int found = 0;
    
    while (position <= file_size - marker_size) {
        fseek(file, position, SEEK_SET);
        size_t read = fread(check_buffer, 1, marker_size, file);
        
        if (read != marker_size) { break; } // Error or EOF
        
        if (memcmp(check_buffer, marker, marker_size) == 0) { // Check if marker matches
            found = 1;
            position += marker_size; // Move past the marker
            break;
        }
        
        position++; // Move forward one byte at a time
    }
    
    free(check_buffer);
    
    if (!found) { fclose(file); return result; }
    
    size_t remaining_size = file_size - position; // Calculate size after marker
    
    unsigned char* data_buffer = malloc(remaining_size);
    if (!data_buffer) { fclose(file); return result; }
    
    fseek(file, position, SEEK_SET); // Load data after marker
    size_t bytes_read = fread(data_buffer, 1, remaining_size, file);
    
    fclose(file);
    
    if (bytes_read != remaining_size) { free(data_buffer); return result; }
    
    result.data = data_buffer;
    result.size = remaining_size;
    
    return result;
}

MarkerData load_ft_structure(const char* filename) {
    return MarkerData load_after_marker(filename, FILE_TAGS_STRUCTURE_MARKER, FILE_TAGS_STRUCTURE_MARKER_SIZE);
}