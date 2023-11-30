#include "files-list.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

/*!
 * @brief clear_files_list clears a files list
 * @param list is a pointer to the list to be cleared
 * This function is provided, you don't need to implement nor modify it
 */
void clear_files_list(files_list_t *list) {
    while (list->head) {
        files_list_entry_t *tmp = list->head;
        list->head = tmp->next;
        free(tmp);
    }
}

/*!
 *  @brief add_file_entry adds a new file to the files list.
 *  It adds the file in an ordered manner (strcmp) and fills its properties
 *  by calling stat on the file.
 *  If the file already exists, it does nothing and returns NULL.
 *  @param list the list to add the file entry into
 *  @param file_path the full path (from the root of the considered tree) of the file
 *  @return a pointer to the added entry if success, NULL else (out of memory or file already exists)
 */
files_list_entry_t *add_file_entry(files_list_t *list, char *file_path) {
    // Create a new entry
    files_list_entry_t *new_entry = (files_list_entry_t *)malloc(sizeof(files_list_entry_t));
    if (!new_entry) {
        return NULL; // Out of memory
    }

    // Fill properties using stat on the file
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        // Unable to get file stat, free the new entry and return NULL
        free(new_entry);
        return NULL;
    }

    // Copy the file path to the entry
    strncpy(new_entry->path_and_name, file_path, sizeof(new_entry->path_and_name) - 1);
    new_entry->path_and_name[sizeof(new_entry->path_and_name) - 1] = '\0';

    // Fill other properties
    new_entry->mtime = file_stat.st_mtim;
    new_entry->size = (uint64_t)file_stat.st_size;
    // You need to implement code for md5sum, entry_type, and mode

    // Add the entry to the list in an ordered manner
    if (!list->head || strcmp(new_entry->path_and_name, list->head->path_and_name) < 0) {
        // Insert at the beginning
        new_entry->next = list->head;
        new_entry->prev = NULL;
        if (list->head) {
            list->head->prev = new_entry;
        } else {
            // Empty list
            list->tail = new_entry;
        }
        list->head = new_entry;
    } else {
        // Insert in the middle or at the end
        files_list_entry_t *cursor = list->head;
        while (cursor->next && strcmp(new_entry->path_and_name, cursor->next->path_and_name) >= 0) {
            cursor = cursor->next;
        }

        if (cursor->next && strcmp(new_entry->path_and_name, cursor->next->path_and_name) == 0) {
            // File already exists, free the new entry and return NULL
            free(new_entry);
            return NULL;
        }

        new_entry->next = cursor->next;
        new_entry->prev = cursor;
        if (cursor->next) {
            cursor->next->prev = new_entry;
        } else {
            // Insert at the end
            list->tail = new_entry;
        }
        cursor->next = new_entry;
    }

    return new_entry;
}

/*!
 * @brief add_entry_to_tail adds an entry directly to the tail of the list
 * It supposes that the entries are provided already ordered, e.g. when a lister process sends its list's
 * elements to the main process.
 * @param list is a pointer to the list to which to add the element
 * @param entry is a pointer to the entry to add. The list becomes owner of the entry.
 * @return 0 in case of success, -1 else
 */
int add_entry_to_tail(files_list_t *list, files_list_entry_t *entry) {
    if (!list || !entry) {
        return -1;
    }

    if (!list->head) {
        // Empty list, add the entry as the head
        list->head = entry;
        list->tail = entry;
    } else {
        // Add the entry to the tail
        entry->prev = list->tail;
        entry->next = NULL;
        list->tail->next = entry;
        list->tail = entry;
    }

    return 0;
}

/*!
 *  @brief find_entry_by_name looks up for a file in a list
 *  The function uses the ordering of the entries to interrupt its search
 *  @param list the list to look into
 *  @param file_path the full path of the file to look for
 *  @param start_of_src the position of the name of the file in the source directory (removing the source path)
 *  @param start_of_dest the position of the name of the file in the destination dir (removing the dest path)
 *  @return a pointer to the element found, NULL if none were found.
 */
files_list_entry_t *find_entry_by_name(files_list_t *list, char *file_path, size_t start_of_src, size_t start_of_dest) {
    if (!list || !file_path) {
        return NULL;
    }

    // Implement the search based on the ordering of entries
    for (files_list_entry_t *cursor = list->head; cursor != NULL; cursor = cursor->next) {
        // You need to implement the comparison based on file_path, start_of_src, and start_of_dest
        if (strcmp(cursor->path_and_name + start_of_src, file_path + start_of_dest) == 0) {
            // Entry found
            return cursor;
        }
    }

    return NULL; // Not found
}

/*!
 * @brief display_files_list displays a files list
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list(files_list_t *list) {
    if (!list)
        return;

    for (files_list_entry_t *cursor = list->head; cursor != NULL; cursor = cursor->next) {
        printf("%s\n", cursor->path_and_name);
    }
}

/*!
 * @brief display_files_list_reversed displays a files list from the end to the beginning
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list_reversed(files_list_t *list) {
    if (!list)
        return;

    for (files_list_entry_t *cursor = list->tail; cursor != NULL; cursor = cursor->prev) {
        printf("%s\n", cursor->path_and_name);
    }
}