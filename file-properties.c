#include "file-properties.h"

#include <sys/stat.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "defines.h"
#include <fcntl.h>
#include <stdio.h>
#include "utility.h"

/*!
 * @brief get_file_stats gets all of the required information for a file (inc. directories)
 * @param entry the files list entry
 * You must get:
 * - for files:
 *   - mode (permissions)
 *   - mtime (in nanoseconds)
 *   - size
 *   - entry type (FICHIER)
 *   - MD5 sum
 * - for directories:
 *   - mode
 *   - entry type (DOSSIER)
 * @return -1 in case of error, 0 else
 */
int get_file_stats(files_list_entry_t *entry) {
    // Check for NULL entry
    if (!entry) {
        return -1;
    }

    struct stat file_stat;
    if (stat(entry->path_and_name, &file_stat) != 0) {
        // Error getting file stat
        return -1;
    }

    // Fill properties for both files and directories
    entry->mode = file_stat.st_mode;
    entry->mtime = file_stat.st_mtim;
    entry->entry_type = S_ISDIR(file_stat.st_mode) ? DOSSIER : FICHIER;

    if (entry->entry_type == FICHIER) {
        // Additional properties for files
        entry->size = (uint64_t)file_stat.st_size;

        // Call the function to compute MD5 sum
        if (compute_file_md5(entry) != 0) {
            // Error computing MD5
            return -1;
        }
    }

    return 0; // Success
}

/*!
 * @brief compute_file_md5 computes a file's MD5 sum
 * @param entry the pointer to the files list entry
 * @return -1 in case of error, 0 else
 * Use libcrypto functions from openssl/evp.h
 */
int compute_file_md5(files_list_entry_t *entry) {
    // Check for NULL entry or if it's not a file
    if (!entry || entry->entry_type != FICHIER) {
        return -1;
    }

    FILE *file = fopen(entry->path_and_name, "rb");
    if (!file) {
        // Error opening the file
        return -1;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        // Error creating MD context
        fclose(file);
        return -1;
    }

    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    EVP_DigestInit(mdctx, EVP_md5());

    while (1) {
        unsigned char data[1024];
        size_t read_len = fread(data, 1, sizeof(data), file);

        if (read_len > 0) {
            EVP_DigestUpdate(mdctx, data, read_len);
        }

        if (read_len < sizeof(data)) {
            break;
        }
    }

    EVP_DigestFinal(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);
    fclose(file);

    // Convert MD5 to string
    char md5_str[(EVP_MAX_MD_SIZE * 2) + 1];
    for (unsigned int i = 0; i < md_len; i++) {
        sprintf(&md5_str[i * 2], "%02x", md_value[i]);
    }
    md5_str[md_len * 2] = '\0';

    // Copy MD5 to the entry
    strncpy((char *)entry->md5sum, md5_str, sizeof(entry->md5sum) - 1);
    entry->md5sum[sizeof(entry->md5sum) - 1] = '\0';

    return 0; // Success
}

/*!
 * @brief directory_exists tests the existence of a directory
 * @param path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    struct stat dir_stat;
    return stat(path_to_dir, &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode);
}

/*!
 * @brief is_directory_writable tests if a directory is writable
 * @param path_to_dir the path to the directory to test
 * @return true if dir is writable, false else
 * Hint: try to open a file in write mode in the target directory.
 */
bool is_directory_writable(char *path_to_dir) {
    // Try to open a file in write mode in the target directory
    int fd = open(concat_path(path_to_dir, "tempfile", ""), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd != -1) {
        close(fd);
        remove(concat_path(path_to_dir, "tempfile", ""));  // Remove the temporary file
        return true; // Directory is writable
    }
    return false; // Directory is not writable
}
