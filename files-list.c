#include "files-list.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <sys/stat.h>

files_list_entry_t *add_file_entry(files_list_t *list, char *file_path) {
    // Check if the file entry already exists
    files_list_entry_t *existing_entry = find_entry_by_name(list, file_path, 0, 0);
    if (existing_entry != NULL) {
        return NULL; // Entry already exists
    }

    // Create a new file entry
    files_list_entry_t *new_entry = (files_list_entry_t *)malloc(sizeof(files_list_entry_t));
    if (!new_entry) {
        return NULL; // Memory allocation failed
    }

    // Initialize the new entry's properties
    strcpy(new_entry->path_and_name, file_path);

    // Call stat on the file and fill other properties
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        // Failed to get file information
        free(new_entry);
        return NULL;
    }

    new_entry->mtime = file_stat.st_mtim;
    new_entry->size = file_stat.st_size;
    new_entry->entry_type = S_ISDIR(file_stat.st_mode) ? DOSSIER : FICHIER;
    new_entry->mode = file_stat.st_mode;

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
        new_entry->prev = NULL;
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
        }

        current = current->next;
    }

    return NULL; // Entry not found
}

void display_files_list(files_list_t *list) {
    if (!list) {
        return;
    }

    for (files_list_entry_t *cursor = list->head; cursor != NULL; cursor = cursor->next) {
        printf("%s\n", cursor->path_and_name);
    }
}

void display_files_list_reversed(files_list_t *list) {
    if (!list) {
        return;
    }

    for (files_list_entry_t *cursor = list->tail; cursor != NULL; cursor = cursor->prev) {
        printf("%s\n", cursor->path_and_name);
    }
}

void clear_files_list(files_list_t *list) {
    while (list->head) {
        files_list_entry_t *tmp = list->head;
        list->head = tmp->next;
        free(tmp);
    }
}
