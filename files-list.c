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
 *  It adds the file in an ordered manner (strcmp) without filling its properties.
 *  If the file already exists, it does nothing and returns NULL
 *  @param list the list to add the file entry into
 *  @param file_path the full path (from the root of the considered tree) of the file
 *  @return a pointer to the added element if success, NULL else (out of memory or duplicate entry)
 */
files_list_entry_t *add_file_entry(files_list_t *list, char *file_path) {
    // Check if the file entry already exists
    if (find_entry_by_name(list, file_path, 0, 0) != NULL) {
        return NULL; // Entry already exists
    }

    // Create a new file entry
    files_list_entry_t *new_entry = malloc(sizeof(files_list_entry_t));
    if (!new_entry) {
        return NULL; // Memory allocation failed
    }

    // Initialize the new entry's properties
    strncpy(new_entry->path_and_name, file_path, sizeof(new_entry->path_and_name));
    new_entry->next = NULL;
    new_entry->prev = NULL;

    // Add the new entry to the list in an ordered manner
    files_list_entry_t *current = list->head;
    files_list_entry_t *prev = NULL;
    while (current != NULL && strcmp(file_path, current->path_and_name) > 0) {
        prev = current;
        current = current->next;
    }

    if (prev == NULL) {
        // Insert at the beginning of the list
        new_entry->next = list->head;
        if (list->head != NULL) {
            list->head->prev = new_entry;
        }
        list->head = new_entry;
    } else {
        // Insert in the middle or at the end of the list
        new_entry->next = current;
        new_entry->prev = prev;
        prev->next = new_entry;
        if (current != NULL) {
            current->prev = new_entry;
        }
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
        return -1; // Invalid input parameters
    }

    // Update the entry's pointers
    entry->prev = list->tail;
    entry->next = NULL;

    // Update the list's pointers
    if (list->tail != NULL) {
        list->tail->next = entry;
    }

    list->tail = entry;

    // If the list was empty, update the head pointer
    if (list->head == NULL) {
        list->head = entry;
    }

    return 0; // Success
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
        return NULL; // Invalid input parameters
    }

    files_list_entry_t *current = list->head;

    // Iterate through the list
    while (current != NULL) {
        // Implement the logic to compare file paths based on your requirements
        // You may want to use start_of_src and start_of_dest
        if (strcmp(current->path_and_name, file_path) == 0) {
            return current; // Entry found
        } else if (strcmp(current->path_and_name, file_path) > 0) {
            // Entry not found because we passed the point where it could be
            break;
        }

        current = current->next;
    }

    return NULL; // Entry not found
}

/*!
 * @brief display_files_list displays a files list
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list(files_list_t *list) {
    if (!list) {
        return;
    }

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
    if (!list) {
        return;
    }

    for (files_list_entry_t *cursor = list->tail; cursor != NULL; cursor = cursor->prev) {
        printf("%s\n", cursor->path_and_name);
    }
}
