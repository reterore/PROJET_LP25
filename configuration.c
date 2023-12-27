#include "configuration.h"
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

typedef enum { DATE_SIZE_ONLY, NO_PARALLEL, DRY_RUN, VERBOSE } long_opt_values;

/*!
 * @brief function display_help displays a brief manual for the program usage
 * @param my_name is the name of the binary file
 * This function is provided with its code; you don't have to implement nor modify it.
 */
void display_help(char *my_name) {
    printf("%s [options] source_dir destination_dir\n", my_name);
    printf("Options: \t-n <processes count>\tnumber of processes for file calculations\n");
    printf("         \t-h display help (this text)\n");
    printf("         \t--date_size_only disables MD5 calculation for files\n");
    printf("         \t--no-parallel disables parallel computing (cancels values of option -n)\n");
    printf("         \t--dry-run perform a trial run with no changes made\n");
    printf("         \t-v enable verbose mode\n");
}

/*!
 * @brief init_configuration initializes the configuration with default values
 * @param the_config is a pointer to the configuration to be initialized
 */
void init_configuration(configuration_t *the_config) {
    if (the_config != NULL) {
        // Initialize the configuration with default values
        memset(the_config, 0, sizeof(configuration_t)); // Clear the entire structure
        the_config->processes_count = 1; // Default to a single process
        the_config->is_parallel = true; // Default to parallel computing
        the_config->uses_md5 = true; // Default to calculating MD5 for files
        the_config->uses_dry_run = false; // Default to not performing a dry run
        the_config->verbose = false; // Default to non-verbose mode
    }
}

/*!
 * @brief set_configuration updates a configuration based on options and parameters passed to the program CLI
 * @param the_config is a pointer to the configuration to update
 * @param argc is the number of arguments to be processed
 * @param argv is an array of strings with the program parameters
 * @return -1 if configuration cannot succeed, 0 when ok
 */
int set_configuration(configuration_t *the_config, int argc, char *argv[]) {
    if (the_config == NULL) {
        return -1;
    }

    int opt;
    int long_index = 0;
    static struct option long_options[] = {
            {"date_size_only", no_argument, NULL, DATE_SIZE_ONLY},
            {"no-parallel", no_argument, NULL, NO_PARALLEL},
            {"dry-run", no_argument, NULL, DRY_RUN},
            {"verbose", no_argument, NULL, VERBOSE},
            {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "hn:v", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'n':
                the_config->processes_count = atoi(optarg);
                if (the_config->processes_count <= 0) {
                    fprintf(stderr, "Error: Invalid number of processes.\n");
                    return -1;
                }
                break;
            case 'h':
                display_help(argv[0]);
                exit(EXIT_SUCCESS);
            case 'v':
                the_config->verbose = true;
                break;
            case DATE_SIZE_ONLY:
                the_config->uses_md5 = false;
                break;
            case NO_PARALLEL:
                the_config->is_parallel = false;
                the_config->processes_count = 1; // Reset process count if parallel is disabled
                break;
            case DRY_RUN:
                the_config->uses_dry_run = true;
                break;
            case VERBOSE:
                the_config->verbose = true;
                break;
            default:
                return -1;
        }
    }

    // Check for the remaining non-option arguments (source_dir and destination_dir)
    if (optind + 2 != argc) {
        fprintf(stderr, "Error: Incorrect number of arguments.\n");
        return -1;
    }

    // Set source and destination directories
    strncpy(the_config->source, argv[optind], sizeof(the_config->source) - 1);
    the_config->source[sizeof(the_config->source) - 1] = '\0';

    strncpy(the_config->destination, argv[optind + 1], sizeof(the_config->destination) - 1);
    the_config->destination[sizeof(the_config->destination) - 1] = '\0';

    return 0;
}
