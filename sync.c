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
 * @brief make_files_list buils a files list in no parallel mode
 * @param list is a pointer to the list that will be built
 * @param target_path is the path whose files to list
 */
void make_files_list(files_list_t *list, char *target_path) {
    if (list == NULL || target_path == NULL) {
        return;
    }

    make_list(list, target_path);

    files_list_entry_t *p_entry = list->head;
    while (p_entry != NULL) {
        get_file_stats(p_entry);
        p_entry = p_entry->next;
    }
}

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
    char dest_entry_path[PATH_SIZE]  = "";
    concat_path(dest_entry_path, the_config->destination, source_entry->path_and_name + strlen(the_config->source));

    if (the_config->dry_run == true) {
        printf("%s copied to %s.\n", source_entry->path_and_name, dest_entry_path);
        return;
    }

    // open the source file for reading
    int source_file = open(source_entry->path_and_name, O_RDONLY);
    if (source_file == -1) {
        fprintf(stderr, "Error opening source file");
        return;
    }

    // open the destination file
    int destination_file = open(dest_entry_path, O_WRONLY | O_CREAT | O_TRUNC, source_entry->mode);
    // O_WRONLY: fichier doit être ouvert en mode écriture seulement
    // O_CREAT: crée le fichier s'il n'existe pas
    // O_TRUNC: tronque le fichier à zéro s'il existe
    if (destination_file == -1) {
        fprintf(stderr, "Error opening destination file");
        close(source_file);
        return;
    }

    // copie des infos du fichier
    off_t offset = 0;
    ssize_t bytes_copied = sendfile(destination_file, source_file, &offset, source_entry->size);

    if (bytes_copied == -1) {
        fprintf(stderr, "Error copying file");
    } else {
        if (the_config->verbose == true) {
            printf("%s copied to %s.\n", source_entry->path_and_name, dest_entry_path);
        }
        struct timespec new_time[2];
        new_time[0].tv_nsec = UTIME_NOW;
        new_time[0].tv_sec = UTIME_NOW;
        new_time[1].tv_nsec = source_entry->mtime.tv_nsec;
        new_time[1].tv_sec = source_entry->mtime.tv_sec;
        if (utimensat(AT_FDCWD, dest_entry_path, new_time, 0) != 0) {
            fprintf(stderr, "Erreur lors de la modification de l'heure de modification");
        }

        chmod(dest_entry_path, source_entry->mode);
    }

    close(source_file);
    close(destination_file);

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