#include "defines.h"
#include <string.h>

char *concat_path(char *result, char *prefix, char *suffix) {
    // Check for NULL pointers
    if (result == NULL || prefix == NULL || suffix == NULL) {
        return NULL;
    }

    // Copy the prefix to the result
    strcpy(result, prefix);

    // Check if the prefix ends with '/'
    size_t prefix_len = strlen(result);
    if (prefix_len > 0 && result[prefix_len - 1] != '/') {
        // Add a '/' to the end of the prefix
        if (prefix_len + 2 <= PATH_SIZE) {
            result[prefix_len] = '/';
            result[prefix_len + 1] = '\0';
        } else {
            // Not enough space in result
            return NULL;
        }
    }

    // Concatenate the suffix to the result
    size_t suffix_len = strlen(suffix);
    if (prefix_len + suffix_len + 2 <= PATH_SIZE) {
        strcat(result, suffix);
        return result;
    } else {
        // Not enough space in result
        return NULL;
    }
}
