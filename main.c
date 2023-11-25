#include <stdio.h>
#include <assert.h>
#include "sync.h"
#include "configuration.h"
#include "file-properties.h"
#include "processes.h"
#include <unistd.h>
#include <stdlib.h>

// Définir une structure pour les informations sur les fichiers, LISTE CHAINEE
typedef struct FileNode {
    char path[256];
    // POUR PLUS TARD : Ajouter d'autres propriétés de fichiers (par exemple, taille, dates, MD5)
    struct FileNode* next;
} FileNode;

// Fonction d'analyse des options du programm
void parseOptions(int argc, char *argv[], char *sourceFolder, char *destFolder, int *dateSizeOnly, int *numAnalyzers, int *noParallel, int *verbose, int *dryRun) {
    int opt;

    // Set default values
    *dateSizeOnly = 0;
    *numAnalyzers = 1;
    *noParallel = 0;
    *verbose = 0;
    *dryRun = 0;

    while ((opt = getopt(argc, argv, "n:v")) != -1) {
        switch (opt) {
            case 'n':
                *numAnalyzers = atoi(optarg);
                break;
            case 'v':
                *verbose = 1;
                break;
                // Ajouter d'autres cas pour d'autres options si nécessaire
            default:
                fprintf(stderr, "Usage: %s [-n num] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Définir les dossiers source et destination
    if (optind + 2 != argc) {
        fprintf(stderr, "Usage: %s [-n num] [-v] source destination\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(sourceFolder, argv[optind]);
    strcpy(destFolder, argv[optind + 1]);
}

// Fonction d'impression de la liste des fichiers :  parcourt la liste et imprime le chemin de chaque fichier.
void printFileList(FileNode *head) {
    FileNode *current = head;
    while (current != NULL) {
        printf("%s\n", current->path);
        current = current->next;
    }
}

// Fonction permettant de libérer la mémoire allouée à la liste de fichiers : parcourt la liste et libère la mémoire allouée pour chaque nœud de fichier.
void freeFileList(FileNode *head) {
    FileNode *current = head;
    FileNode *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}
/*!
 * @brief main function, calling all the mechanics of the program
 * @param argc its number of arguments, including its own name
 * @param argv the array of arguments
 * @return 0 in case of success, -1 else
 * Function is already provided with full implementation, you **shall not** modify it.
 */
int main(int argc, char *argv[]) {
    // Check parameters:
    // - source and destination are provided
    // - source exists and can be read
    // - destination exists and can be written OR doesn't exist but can be created
    // - other options with getopt (see instructions)


// Valeurs par défaut des options
    char sourceFolder[256];
    char destFolder[256];
    int dateSizeOnly = 0;
    int numAnalyzers = 1;
    int noParallel = 0;
    int verbose = 0;
    int dryRun = 0;

// Analyse des options du programme
    parseOptions(argc, argv, sourceFolder, destFolder, &dateSizeOnly, &numAnalyzers, &noParallel, &verbose, &dryRun);

// Emplacement pour la liste chaînée des fichiers du dossier SOURCE
    FileNode *sourceList = NULL;

// Emplacement pour la liste chaînée des fichiers du dossier de DESTINATION
    FileNode *destList = NULL;

// Placeholder pour le traitement des dossiers source et destination
//

    // Print la liste des fichiers pour les test
    printf("Source Files:\n");
    printFileList(sourceList);

    printf("\nDestination Files:\n");
    printFileList(destList);

    // Libère la mémoire allouée pour les listes de fichiers
    freeFileList(sourceList);
    freeFileList(destList);
    configuration_t my_config;
    init_configuration(&my_config);
    if (set_configuration(&my_config, argc, argv) == -1) {
        return -1;
    }

// Vérifier les répertoires
    if (!directory_exists(my_config.source) || !directory_exists(my_config.destination)) {
        printf("Le répertoire source ou le répertoire de destination n'existe pas\nAbandon\n");
        return -1;
    }
// La destination est-elle accessible en écriture ?
    if (!is_directory_writable(my_config.destination)) {
        printf("Le répertoire de destination %s n'est pas accessible en écriture\n", my_config.destination);
        return -1;
    }

// Préparation (fork, MQ) si parallèle
    process_context_t processes_context;
    prepare(&my_config, &processes_context);

// Exécuter la synchronisation :
    synchronize(&my_config, &processes_context);

    // Clean resources
    clean_processes(&my_config, &processes_context);

    return 0;
}
