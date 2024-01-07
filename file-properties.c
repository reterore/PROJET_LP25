#include "file-properties.h"
#include <sys/stat.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <assert.h>
#include "defines.h"
#include <fcntl.h>
#include <stdio.h>
#include "utility.h"
#include <errno.h>
#include <string.h>

/*!
 * @brief Gets all of the required information for a file (including directories).
 *
 * This function retrieves information such as mode (permissions), mtime (in nanoseconds),
 * size, entry type (FICHIER), and MD5 sum.
 *
 * For directories, it obtains mode and entry type (DOSSIER).
 *
 * @param entry The files list entry.
 * @return -1 in case of error, 0 otherwise.
 */
int get_file_stats(files_list_entry_t *entry) {
    struct stat sb;
    const char *path = entry->path_and_name;

    int result = lstat(path, &sb);
    if (result == -1) {
        printf("%s\n", strerror(errno));
        // Handle the error...
    }

    entry->mtime.tv_sec = sb.st_mtim.tv_sec;  // Copy seconds
    entry->mtime.tv_nsec = sb.st_mtim.tv_nsec;
    entry->size = sb.st_size;
    entry->mode = sb.st_mode;

    if (S_ISDIR(sb.st_mode)) {
        entry->entry_type = DOSSIER;
    } else if (S_ISREG(sb.st_mode)) {
        entry->entry_type = FICHIER;

        // Utilisation de concat_path pour ajouter "example.txt" au chemin
        char result[PATH_SIZE];
        if (concat_path(result, entry->path_and_name, "example.txt") != NULL) {
            // Maintenant, 'result' contient le chemin complet avec le nouveau nom de fichier
            // Utilisez 'result' comme nécessaire.
            printf("Full path with new filename: %s\n", result);
        } else {
            // Gestion des erreurs lors de l'utilisation de concat_path
            fprintf(stderr, "Error in concat_path\n");
        }

        assert(compute_file_md5(entry) == 0 && "Error computing MD5");
    } else {
        return -1; // Type de fichier inconnu
    }
    return 0;
}

/*!
 * @brief Computes a file's MD5 sum using libcrypto functions from openssl/evp.h.
 *
 * @param entry The pointer to the files list entry.
 * @return -1 in case of error, 0 otherwise.
 */
int compute_file_md5(files_list_entry_t *entry) {
    // Open and check if the file has been opened correctly.
    int file = open(entry->path_and_name, O_RDONLY);
    assert(file != -1 && "Unable to open the file");

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_md5();

    assert(mdctx && md && EVP_DigestInit_ex(mdctx, md, NULL) && "Error in MD5 sum initialization");

    unsigned char buffer[1024];
    ssize_t bytes;
    while ((bytes = read(file, buffer, sizeof(buffer))) > 0) {
        assert(EVP_DigestUpdate(mdctx, buffer, bytes) && "Error in MD5 sum update");
    }

    close(file);

    unsigned int md_len;
    assert(EVP_DigestFinal_ex(mdctx, entry->md5sum, &md_len) && "Error in MD5 sum finalization");

    EVP_MD_CTX_free(mdctx);

    return 0;
}

/*!
 * @brief Tests the existence of a directory.
 *
 * @param path_to_dir A string with the path to the directory.
 * @return true if the directory exists, false otherwise.
 */
bool directory_exists(char *path_to_dir) {
    struct stat sb;
    if (stat(path_to_dir, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        return true;
    } else {
        return false;
    }
}

/*!
 * @brief Tests if a directory is writable.
 *
 * @param path_to_dir The path to the directory to test.
 * @return true if the directory is writable, false otherwise.
 * @note Try to open a file in write mode in the target directory.
 */
bool is_directory_writable(char *path_to_dir) {
    // Check if the directory is readable
    DIR *dir = opendir(path_to_dir);
    if (!dir) {
        perror("Unable to open the directory");
        return false;
    }

    closedir(dir);

    // Try to write to the directory using access
    if (access(path_to_dir, W_OK) == 0) {
        return true;
    } else {
        return false;
    }
}
