/*  TODO:
==========
1. Implement Semaphores
     i. Enforce SEM_FULL in producer
    ii. Enforce SEM_MUTEX for shared memory access.
2. 
*/

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <bits/stdc++.h>

#include "formatting.hpp"
#include "buffer_queue.hpp"

using namespace std;

// Protoypes of testing methods
void print_buffer( pair<short int, double> *);


/*  ======================================================= Global Variables =======================================================  */
/* Arguments */
string COMMODITY_NAME;
double MEAN, STDEV, INTERVAL_MS;

/* Shared memory */
int BUFFER_SIZE;
int SHMID; // ID used to access shared memory segment

//* Semaphores */
int SEMID; // ID used to access semaphore set
const int SEM_MUTEX = 0; // Semaphore to enforce mutual exclusion
const int SEM_FULL = 1; // Semaphore to check if buffer is full
const int SEM_EMPTY = 2; // Semaphore to check if buffer is empty
short int SEM_COUNT = 3;

/* Miscellanous */
default_random_engine RANDOM_GENERATOR;
normal_distribution<double> NORMAL_DISTRIBUTION;
struct shmid_ds SHM_STATS;
int RANDOM_ID;
const char *FILENAME_SHM = "shmfile", *FILENAME_SEM = "semfile";
unordered_map<string, short int> COMMODITY_NAME_TO_ID =
    {
        {"ALUMINIUM", 0}, {"COPPER", 1}, {"COTTON", 2}, {"CRUDEOIL", 3}, {"GOLD", 4}, {"LEAD", 5},
        {"MENTHAOIL", 6}, {"NATURALGAS", 7}, {"NICKEL", 8}, {"SILVER", 9}, {"ZINC", 10}
    };

/*  =======================================================     Functions    =======================================================  */
/// @brief Function to get unique key
key_t get_unique_key(const char *__filename)
{
    key_t unique_key = -1;
    do {
        unique_key = ftok(FILENAME_SEM, RANDOM_ID);
        RANDOM_ID = ( unique_key == -1 ? rand() : RANDOM_ID) ;
    } while( unique_key == -1 );
    return unique_key;
}

/// @brief  Function to get ID of semaphore group
int get_semid()
{
    key_t semkey = get_unique_key(FILENAME_SEM);
    return semget(semkey, SEM_COUNT, IPC_CREAT | 0666);
}

/// @brief  Function to get ID for shared memory to be used by the consumer
int get_shmid()
{
    key_t shmkey = get_unique_key(FILENAME_SHM);
    return shmget(shmkey, (BUFFER_SIZE+1) * sizeof( pair<short int, double> ), 0);
}

/// @brief Function to initialize producer
bool setup_producer(int argument_count, char *arguments[])
{
    if (argument_count <= 4)
        return cerr << SET_COLOR(COLOR_RED) << "Insufficient arguments" << REMOVE_ALL_FORMATTING() << endl, false;

    COMMODITY_NAME  = arguments[1];
    MEAN            = atof( arguments[2] );
    double variance = atof( arguments[3] );
    INTERVAL_MS     = atof( arguments[4] );
    BUFFER_SIZE     = atoi( arguments[5] );
    STDEV = variance * variance;

    if( (SHMID = get_shmid()) == -1 )
        return cerr << SET_COLOR( COLOR_RED ) << "Error allocating shared memory" << REMOVE_ALL_FORMATTING() << flush << endl, false;

    if( (SEMID = get_semid()) == -1 )
        return cerr << SET_COLOR( COLOR_RED ) << "Error getting semaphores" << REMOVE_ALL_FORMATTING() << endl, false;

    if( ! COMMODITY_NAME_TO_ID.count( COMMODITY_NAME ) )
        return cerr << SET_COLOR( COLOR_RED ) << "Invalid commodity name (argument 1)" << REMOVE_ALL_FORMATTING() << endl, false;

    if( MEAN <= 0 )
        return cerr << SET_COLOR( COLOR_RED ) << "Command expects positive-valued mean (argument 2)" << REMOVE_ALL_FORMATTING() << endl, false;

    if( variance <= 0 )
        return cerr << SET_COLOR( COLOR_RED ) << "Command expects positive-valued variance (argument 3)" << REMOVE_ALL_FORMATTING() << endl, false;

    if( INTERVAL_MS <= 0 )
        return cerr << SET_COLOR( COLOR_RED ) << "Command expects positive-valued interval_ms (argument 4)" << REMOVE_ALL_FORMATTING() << endl, false;

    if( BUFFER_SIZE <= 0 )
        return cerr << SET_COLOR( COLOR_RED ) << "Invalid buffer size (argument 5)" << REMOVE_ALL_FORMATTING() << endl, false;

    normal_distribution<double> distribution( MEAN, STDEV );
    NORMAL_DISTRIBUTION = distribution;

    return true;
}

/// @brief Function to generate random price by normal distribution
double generate_price()
{
    return NORMAL_DISTRIBUTION( RANDOM_GENERATOR );
}

/// @brief Function to add to shared memory buffer
void add_to_buffer(short commodity_ID, double commodity_price)
{
}

/// @brief SIGINT handler to destroy shared memory if no other processes are attached to it
void terminate_process(int sig_num)
{
    cout << "\r" << SET_COLOR( COLOR_PURPLE ) << COMMODITY_NAME << " producer terminated." << REMOVE_ALL_FORMATTING() << endl;
    exit( EXIT_SUCCESS );
}

/*  ================================================================================================================================  */
/*  =======================================================    SEMAPHORES    =======================================================  */
/*  ================================================================================================================================  */
void sem_signal( int __semid, const int __semnum )
{
    struct sembuf sem_buf;
    sem_buf.sem_op = 1;
    sem_buf.sem_num = __semnum;
    sem_buf.sem_flg = 0;
    semop( __semid, &sem_buf, 1 );
}

void sem_wait( int __semid, const int __semnum )
{
    struct sembuf sem_buf;
    sem_buf.sem_op = -1;
    sem_buf.sem_num = __semnum;
    sem_buf.sem_flg = 0;
    semop( __semid, &sem_buf, 1 );
}

/*  ================================================================================================================================  */
/*  =======================================================       TIME       =======================================================  */
/*  ================================================================================================================================  */
void log_event( string msg )
{
    struct timespec ts;
    timespec_get( &ts, TIME_UTC);
    char buff[100];
    strftime( buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec) );
    cerr << SET_COLOR( COLOR_CYAN ) << COMMODITY_NAME << " Producer => " << REMOVE_ALL_FORMATTING();
    fprintf( stderr, "[%s.%09ld] : %s\n", buff, (intmax_t)ts.tv_nsec, msg.c_str() );
}

/*  ================================================================================================================================  */
/*  =======================================================       MAIN       =======================================================  */
/*  ================================================================================================================================  */
int main(int argc, char *argv[])
{
    signal(SIGINT, terminate_process);
    if (!setup_producer(argc, argv))
        return EXIT_FAILURE;

    cout << SAVE_CURSOR();
    fflush( stdout );

    short int commodity_id;
    double commodity_price;

    while (true)
    {

        log_event( "Generating new price" );
        commodity_id = COMMODITY_NAME_TO_ID[ COMMODITY_NAME ];
        commodity_price = generate_price();
        pair<short int, double> commodity_data = make_pair( commodity_id, commodity_price );
        log_event( (string)"Generated new price of " + to_string( commodity_price ) );

        log_event( "Waiting for access to shared buffer" );
        sem_wait( SEMID, SEM_EMPTY );
        sem_wait( SEMID, SEM_MUTEX );
        log_event( "Access to shared memory granted" );
        /* Begin Production */

        pair<short int, double> *buf;
        auto *shm_ptr = shmat(SHMID, (void *)0, 0);
        if( shm_ptr == (void*)-1 )
        {
            log_event( SET_COLOR( COLOR_RED ) + "Error occurred during attachment to shared memory" + REMOVE_ALL_FORMATTING() );
            sem_signal( SEMID, SEM_MUTEX );
            log_event( "Unlocked shared memory" );
            terminate_process( 0 );
        }
        
        buf = (pair<short int, double> *) shm_ptr;
        Buffer buffer( buf, BUFFER_SIZE, buf[BUFFER_SIZE].first, buf[BUFFER_SIZE].second );

        if( !buffer.isfull() )
        {
            log_event( "Placing (" + to_string(commodity_price) + ") on shared buffer" );
            buffer.insert( commodity_data );
        }
        else log_event( SET_COLOR( COLOR_RED ) + "Buffer full. Placement unsuccessful" + REMOVE_ALL_FORMATTING() );
        
        sem_signal( SEMID, SEM_MUTEX );
        sem_signal( SEMID, SEM_FULL );
        log_event( "Unlocked shared memory" );

        shmdt( buf ); // Detach from memory block
        /* End Processing */

        usleep(INTERVAL_MS * 1000);
    }

    return EXIT_SUCCESS;
}

/*  ================================================================================================================================  */
/*  =======================================================      TESTING     =======================================================  */
/*  ================================================================================================================================  */
void print_buffer( pair<short int, double> *__buffer )
{
    cout << endl;
    cout << "+----+---------+" << endl;
    cout << "| ID |  PRICE  |" << endl;
    for ( int idx = 0 ; idx < BUFFER_SIZE ; idx ++ )
        printf("| %2d | %7.2lf |\n", __buffer[ idx ].first, __buffer[ idx ].second);
    cout << "+----+---------+" << endl;
}
