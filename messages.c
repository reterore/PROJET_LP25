#include "messages.h"
#include <sys/msg.h>
#include <string.h>

int send_file_entry(int msg_queue, int recipient, files_list_entry_t *file_entry, int cmd_code) {
    // Vérifier les paramètres d'entrée
    if (file_entry == NULL || recipient < 0) {
        // Gestion d'erreur pour les paramètres invalides
        return -1;
    }

    files_list_entry_transmit_t message;
    message.mtype = recipient;
    message.op_code = cmd_code;

    // Utiliser memcpy pour copier la structure complète
    memcpy(&message.payload, file_entry, sizeof(files_list_entry_t));

    // Envoyer le message
    return msgsnd(msg_queue, &message, sizeof(files_list_entry_transmit_t) - sizeof(long), 0);
}

int send_analyze_dir_command(int msg_queue, int recipient, char *target_dir) {
    // Vérifier les paramètres d'entrée
    if (target_dir == NULL || recipient < 0) {
        // Gestion d'erreur pour les paramètres invalides
        return -1;
    }

    analyze_dir_command_t message;
    message.mtype = recipient;
    message.op_code = COMMAND_CODE_ANALYZE_DIR;

    // Utiliser strncpy et s'assurer que la chaîne cible est toujours terminée par '\0'
    strncpy(message.target, target_dir, PATH_SIZE);
    message.target[PATH_SIZE - 1] = '\0';

    // Envoyer le message
    return msgsnd(msg_queue, &message, sizeof(analyze_dir_command_t) - sizeof(long), 0);
}


int send_analyze_file_command(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    return send_file_entry(msg_queue, recipient, file_entry, COMMAND_CODE_ANALYZE_FILE);
}

int send_analyze_file_response(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    return send_file_entry(msg_queue, recipient, file_entry, COMMAND_CODE_FILE_ANALYZED);
}

int send_files_list_element(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    return send_file_entry(msg_queue, recipient, file_entry, COMMAND_CODE_FILE_ENTRY);
}

int send_list_end(int msg_queue, int recipient) {
    simple_command_t message;
    message.mtype = recipient;
    message.message = COMMAND_CODE_LIST_COMPLETE;

    return msgsnd(msg_queue, &message, sizeof(simple_command_t) - sizeof(long), 0);
}

int send_terminate_command(int msg_queue, int recipient) {
    simple_command_t message;
    message.mtype = recipient;
    message.message = COMMAND_CODE_TERMINATE;

    return msgsnd(msg_queue, &message, sizeof(simple_command_t) - sizeof(long), 0);
}

int send_terminate_confirm(int msg_queue, int recipient) {
    simple_command_t message;
    message.mtype = recipient;
    message.message = COMMAND_CODE_TERMINATE_OK;

    return msgsnd(msg_queue, &message, sizeof(simple_command_t) - sizeof(long), 0);
}
