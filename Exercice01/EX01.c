#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#define STDOUT 1
#define STDERR 2

#define MAX_PATH_LENGTH 4096

#define SIZE (int)1e8 // Size of the array
#define NUM_THREADS 1 // Number of threads to use, must be a divisor of SIZE

int tab[SIZE]; // Array to store random values

// Structure to store min and max values
struct ThreadResult
{
    int min;
    int max;
};

// Array to store results from each thread
struct ThreadResult threadResults[NUM_THREADS];
// Mutex to protect the result array
pthread_mutex_t resultMutex = PTHREAD_MUTEX_INITIALIZER;

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
 * Find the min and max values of the array
 * and store them in the min and max variables
 *
 * \return void
 */
void *findMinMaxThread(void *arg)
{
    int thread_id = *((int *)arg); // Get thread ID

    int chunk_size = SIZE / NUM_THREADS;                     // Size of the chunk to process
    int start = thread_id * chunk_size;                      // Start index of the chunk
    int end = (thread_id == NUM_THREADS - 1) ? SIZE : (thread_id + 1) * chunk_size; // End index of the chunk

    struct ThreadResult result;                              // Result of the thread
    result.min = tab[start];                                 // Initialize min and max with the first value of the chunk
    result.max = tab[start];

    // Find min and max values of the chunk and store them in the result
    for (int i = start + 1; i < end; i++)
    {
        if (tab[i] < result.min)
        {
            result.min = tab[i];
        }
        if (tab[i] > result.max)
        {
            result.max = tab[i];
        }
    }

    // Lock the mutex before updating the global result variables
    pthread_mutex_lock(&resultMutex);

    // Update global result variables
    threadResults[thread_id] = result;

    // Unlock the mutex
    pthread_mutex_unlock(&resultMutex);

    return NULL;
}

/**
 * Initialize the array with random values
 *
 * \return void
*/
void initArray()
{
    for (int i = 0; i < SIZE; i++)
    {
        tab[i] = rand();
    }
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

    // Initialize the array with random values
    initArray();

    // Measure time taken to find min and max using threads
    struct timeval start, end;  

    // Get current time and store it in start
    gettimeofday(&start, NULL);

    // Thread IDs
    pthread_t threads[NUM_THREADS]; // Array of threads
    int thread_ids[NUM_THREADS];    // Array of thread IDs

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;  // Set thread ID
        pthread_create(&threads[i], NULL, findMinMaxThread, &thread_ids[i]); // Create thread
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Combine results from all threads
    struct ThreadResult finalResult;
    finalResult.min = threadResults[0].min;
    finalResult.max = threadResults[0].max;

    // Find min and max values of all threads and store them in the final result
    for (int i = 1; i < NUM_THREADS; i++)
    {
        if (threadResults[i].min < finalResult.min)
        {
            finalResult.min = threadResults[i].min;
        }
        if (threadResults[i].max > finalResult.max)
        {
            finalResult.max = threadResults[i].max;
        }
    }

    // Get current time and calculate elapsed time
    gettimeofday(&end, NULL);

    // Calculate the time taken in seconds
    double elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    // Display results
    printf("Number of threads: %d\n", NUM_THREADS);
    printf("Minimum value: %d\n", finalResult.min);
    printf("Maximum value: %d\n", finalResult.max);
    printf("Time taken: %f seconds\n", elapsedTime);

    return EXIT_SUCCESS;
}