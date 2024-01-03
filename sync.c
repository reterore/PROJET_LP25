#include "sync.h"
#include <dirent.h>
#include <string.h>
#include "processes.h"
#include "utility.h"
#include "messages.h"
#include "file-properties.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>


/*!
 * @brief synchronize is the main function for synchronization
 * It will build the lists (source and destination), then make a third list with differences, and apply differences to the destination
 * It must adapt to the parallel or not operation of the program.
 * @param the_config is a pointer to the configuration
 * @param p_context is a pointer to the processes context
 */
void synchronize(configuration_t *the_config, process_context_t *p_context) {
    // Check for NULL pointers
    if (!the_config || !p_context) {
        fprintf(stderr, "Invalid configuration or process context pointers\n");
        return;
    }

    // Extract source and destination paths from configuration
    char *source_path = the_config->source;
    char *dest_path = the_config->destination;

    files_list_t source_list, dest_list;
    source_list.head = source_list.tail = NULL;
    dest_list.head = dest_list.tail = NULL;

    // Build lists
    make_files_list(&source_list, source_path);
    make_files_list(&dest_list, dest_path);

    // Compare and synchronize lists A COMPLETER  !!!!

    // Free allocated memory
    clear_files_list(&source_list);
    clear_files_list(&dest_list);
}

/*!
 * @brief copy_entry_to_destination copies a file from the source to the destination
 * It keeps access modes and mtime (@see utimensat)
 * Pay attention to the path so that the prefixes are not repeated from the source to the destination
 * Use sendfile to copy the file, mkdir to create the directory
 */
void copy_entry_to_destination(files_list_entry_t *source_entry, configuration_t *the_config) {
    if (source_entry == NULL || the_config == NULL) {
        return;
    }

    // Construct the destination path
    char dest_path[PATH_SIZE];
    concat_path(dest_path, the_config->destination, source_entry->path_and_name);

    // Implementation for constructing the destination path

    // Open the source file
    int source_fd = open(source_entry->path_and_name, O_RDONLY);
    if (source_fd == -1) {
        perror("Error opening source file");
        return;
    }

    // Create the destination directory if it doesn't exist
    char dest_dir[PATH_SIZE];
    strcpy(dest_dir, dest_path);
    dirname(dest_dir);  // Extract the directory from the path
    mkdir_p(dest_dir);  // Create the directory recursively if needed

    // Open (or create) the destination file
    int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, source_entry->mode);
    if (dest_fd == -1) {
        perror("Error opening destination file");
        close(source_fd);
        return;
    }

    // Use sendfile to copy the file
    off_t offset = 0;
    ssize_t bytes_sent = sendfile(dest_fd, source_fd, &offset, source_entry->size);

    if (bytes_sent == -1) {
        perror("Error sending file");
    }

    // Set the destination file's modification time
    struct timespec times[2] = {source_entry->mtime, source_entry->mtime};
    utimensat(AT_FDCWD, dest_path, times, 0);

    // Close file descriptors
    close(source_fd);
    close(dest_fd);
}

/*!
 * @brief make_files_list builds a files list in no parallel mode
 * @param list is a pointer to the list that will be built
 * @param target_path is the path whose files to list
 */
void make_files_list(files_list_t *list, char *target_path) {
    // Open the directory
    DIR *dir = opendir(target_path);

    // Check if the directory was opened successfully
    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    // Initialize the list
    list->head = NULL;
    list->tail = NULL;

    // Read the directory entries
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Add the file to the list
            add_file_entry(list, entry->d_name);
        }
    }

    // Close the directory
    closedir(dir);
}

/*!
 * @brief make_files_lists_parallel makes both (src and dest) files list with parallel processing
 * @param src_list is a pointer to the source list to build
 * @param dst_list is a pointer to the destination list to build
 * @param the_config is a pointer to the program configuration
 * @param msg_queue is the id of the MQ used for communication
 */
void make_files_lists_parallel(files_list_t *src_list, files_list_t *dst_list, configuration_t *the_config, int msg_queue) {
    // Implement parallel file list creation if needed
}

/*!
 * @brief make_list lists files in a location (it recurses in directories)
 * It doesn't get files properties, only a list of paths
 * This function is used by make_files_list and make_files_list_parallel
 * @param list is a pointer to the list that will be built
 * @param target is the target dir whose content must be listed
 */
void make_list(files_list_t *list, char *target) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(target)) == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignore current and parent directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build the full path of the file or directory
        char full_path[PATH_MAX];
        concat_path(full_path, target, entry->d_name);

        // Check if the entry is a directory
        if (directory_exists(full_path)) {
            // Recursively list files in the directory
            make_list(list, full_path);
        } else {
            // Add the file path to the list
            add_file_entry(list, entry->d_name);
        }
    }

    closedir(dir);
}

/*!
 * @brief open_dir opens a dir
 * @param path is the path to the dir
 * @return a pointer to a dir, NULL if it cannot be opened
 */
DIR *open_dir(char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Error opening directory");
    }
    return dir;
}

/*!
 * @brief get_next_entry returns the next entry in an already opened dir
 * @param dir is a pointer to the dir (as a result of opendir, @see open_dir)
 * @return a struct dirent pointer to the next relevant entry, NULL if none found (use it to stop iterating)
 * Relevant entries are all regular files and dir, except . and ..
 */
struct dirent *get_next_entry(DIR *dir) {
    if (dir == NULL) {
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            return entry;
        }
    }

    closedir(dir);
    return NULL;
}