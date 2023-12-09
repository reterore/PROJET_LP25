#include "messages.h"
#include <sys/msg.h>
#include <string.h>

int send_file_entry(int msg_queue, int recipient, files_list_entry_t *file_entry, int cmd_code) {
    files_list_entry_transmit_t message;
    message.mtype = recipient;
    message.op_code = cmd_code;
    memcpy(&message.payload, file_entry, sizeof(files_list_entry_t));

    return msgsnd(msg_queue, &message, sizeof(files_list_entry_transmit_t) - sizeof(long), 0);
}

int send_analyze_dir_command(int msg_queue, int recipient, char *target_dir) {
    analyze_dir_command_t message;
    message.mtype = recipient;
    message.op_code = COMMAND_CODE_ANALYZE_DIR;
    strncpy(message.target, target_dir, PATH_SIZE - 1);
    message.target[PATH_SIZE - 1] = '\0';

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
