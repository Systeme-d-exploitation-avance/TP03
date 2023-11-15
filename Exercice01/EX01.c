#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>

#define STDOUT 1
#define STDERR 2

#define MAX_PATH_LENGTH 4096

#define SIZE (int)1e8

int tab[SIZE];

#define USAGE "USAGE: programme qui initialise un grand tableau d\\’entiers avec des valeurs aléatoires, recherche ensuite le minimum et le maximum du tableau et affiche le résultat"
#define USAGE_SYNTAX "[OPTIONS] No parameters needed"
#define USAGE_PARAMS "OPTIONS:\n\
  -v, --verbose : enable *verbose* mode\n\
  -h, --help    : display this help\n\
"

/**
 * Procedure which displays binary usage
 * by printing on stdout all available options
 *
 * \return void
 */
void print_usage(char *bin_name)
{
    dprintf(1, "%s %s\n%s\n\n%s\n", bin_name, USAGE, USAGE_SYNTAX, USAGE_PARAMS);
}

/**
 * Procedure checks if variable must be free
 * (check: ptr != NULL)
 *
 * \param void* to_free pointer to an allocated mem
 * \see man 3 free
 * \return void
 */
void free_if_needed(void *to_free)
{
    if (to_free != NULL)
        free(to_free);
}

/**
 *
 * \see man 3 strndup
 * \see man 3 perror
 * \return
 */
char *dup_optarg_str()
{
    char *str = NULL;

    if (optarg != NULL)
    {
        str = strndup(optarg, MAX_PATH_LENGTH);

        // Checking if ERRNO is set
        if (str == NULL)
            perror(strerror(errno));
    }

    return str;
}

/**
 * Binary options declaration
 * (must end with {0,0,0,0})
 *
 * \see man 3 getopt_long or getopt
 * \see struct option definition
 */
static struct option binary_opts[] =
    {
        {"help", no_argument, 0, 'h'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}};

/**
 * Binary options string
 * (linked to optionn declaration)
 *
 * \see man 3 getopt_long or getopt
 */
const char *binary_optstr = "hvi:o:";

/**
 * Initialize the array with random values
 *
 * \return void
 */
void initializeArray() {
    for (int i = 0; i < SIZE; i++) {
        tab[i] = rand(); // Initialize with random values
    }
}

/**
 * Find the min and max values of the array
 * and store them in the min and max variables
 * 
 * \return void
*/
void findMinMax() {
    int min = tab[0];
    int max = tab[0];

    for (int i = 1; i < SIZE; i++) {
        if (tab[i] < min) {
            min = tab[i];
        }
        if (tab[i] > max) {
            max = tab[i];
        }
    }

    printf("Minimum value: %d\n", min);
    printf("Maximum value: %d\n", max);
}

/**
 * Binary main loop
 *
 * \return 1 if it exit successfully
 */
int main(int argc, char **argv)
{
    /**
     * Binary variables
     * (could be defined in a structure)
     */
    short int is_verbose_mode = 0;

    // Parsing options
    int opt = -1;
    int opt_idx = -1;

    while ((opt = getopt_long(argc, argv, binary_optstr, binary_opts, &opt_idx)) != -1)
    {
        switch (opt)
        {
        case 'v':
            // verbose mode
            is_verbose_mode = 1;
            break;
        case 'h':
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        default:
            break;
        }
    }

    // Business logic
    
    // Initialize array with random values
    initializeArray();

    // Measure time taken to find min and max
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Find and display min and max
    findMinMax();
    gettimeofday(&end, NULL);

    // Calculate the time taken in seconds
    double elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    printf("Time taken: %f seconds\n", elapsedTime);

    return EXIT_SUCCESS;
}