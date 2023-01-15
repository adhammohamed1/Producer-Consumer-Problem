/*  TODO:
==========
1. Implement Semaphores
     i. Complete sem_init()
    ii. Enforce SEM_EMPTY in consumer
2. 
*/

#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <bits/stdc++.h>

#include "formatting.hpp"
#include "buffer_queue.hpp"

#define GREATER 1
#define EQUAL 0
#define LESS -1

using namespace std;

/*  ================================================================================================================================  */
/*  ======================================================= Global Variables =======================================================  */
/*  ================================================================================================================================  */
/* Shared memory */
int BUFFER_SIZE;
int SHMID; // ID used to access shared memory segment

/* Semaphores */
int SEMID; // ID used to access semaphore set
const int SEM_MUTEX = 0; // Semaphore to enforce mutual exclusion
const int SEM_FULL = 1; // Semaphore to check if buffer is full
const int SEM_EMPTY = 2; // Semaphore to check if buffer is empty
short int SEM_COUNT = 3;

/* Miscellanous */
struct shmid_ds SHM_STATS;
int RANDOM_ID;
const char *FILENAME_SHM = "shmfile", *FILENAME_SEM = "semfile";
unordered_map< short int, deque<double> > last_instances; // Hashmap to hold value of last 4 instances of every commodity ( ID --> deque< prices > )
unordered_map< short int, string > COMMODITY_ID_TO_NAME =
{
    {0, "ALUMINIUM    "}, {1, "COPPER       "}, {2, "COTTON       "}, {3, "CRUDEOIL     "}, 
    {4, "GOLD         "}, {5, "LEAD         "}, {6, "MENTHAOIL    "}, {7, "NATURALGAS   "}, 
    {8, "NICKEL       "}, {9, "SILVER       "}, {10, "ZINC         "}
};

/*  ================================================================================================================================  */
/*  ===============================================    Initialization & Termination    =============================================  */
/*  ================================================================================================================================  */
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
    return shmget(shmkey, (BUFFER_SIZE+1) * sizeof( pair<short int, double> ), IPC_CREAT | 0666);
}

/// @brief  Function to initialize shared memory segment
/// @param __c value to fill the memory segment with
bool shm_init(int __c)
{
    pair<short int, double> * shm_ptr = (pair<short int, double> *) shmat(SHMID, (void*)0, 0);
    if(shm_ptr == (void*) -1)
        return false;

    // Initialize memory block to the value of __c
    for( int idx = 0; idx < BUFFER_SIZE; idx ++ )
    {
        shm_ptr[ idx ].first  = __c;
        shm_ptr[ idx ].second = __c;
    }
    // Extra pair after the bloc indicates the head and tail of the queue
    shm_ptr[ BUFFER_SIZE ].first  = 0;
    shm_ptr[ BUFFER_SIZE ].second = 0;
    return true;
}

/// @brief  Function to initialize semaphores
bool sem_init()
{
    SEMID = get_semid();
    semctl( SEMID, SEM_MUTEX, SETVAL, 1 );
    semctl( SEMID, SEM_EMPTY, SETVAL, BUFFER_SIZE );
    semctl( SEMID, SEM_FULL, SETVAL, 0 );
    return true;
}

/// @brief  Function to initialize consumer and shared resources
bool setup(int argument_count, char *arguments[])
{
    if( argument_count < 2 )
        return cerr << SET_COLOR( COLOR_RED ) << "Insufficient argument" << REMOVE_ALL_FORMATTING() << endl, false;

    if( (BUFFER_SIZE = atoi(arguments[1])) <= 0 )
        return cerr << SET_COLOR( COLOR_RED ) << "Invalid buffer size" << REMOVE_ALL_FORMATTING() << endl, false;

    if( (SHMID = get_shmid()) == -1 ) {
        cerr << SET_COLOR( COLOR_RED ) << "Error allocating shared memory" << REMOVE_ALL_FORMATTING() << endl;
        perror( NULL );
        
        return false;
    }
    
    for ( int id = 0 ; id <= 10; id ++ )
        for ( int j = 0 ; j < 4 ; j ++ )
            last_instances[ id ].push_back( 0.0 );

    bool success = true;
    success = success && shm_init( -1 ); // Initialize buffer space to -1
    success = success && sem_init(); // Initialize semaphores
    return success;
}

/// @brief  SIGINT handler to destroy shared memory if no other processes are attached to it
void terminate_process(int sig_num)
{
    /*shmctl( get_shmid(), IPC_STAT, &SHM_STATS ); // Get shared memory segment stats and store them in SHM_STATS
    if( SHM_STATS.shm_nattch <= 1 ) // Check if at most one process is attached to the shared memory segment
    {*/
        /*====================  Destroy Shared Memory Block ====================*/
        cerr << "\r" << SET_COLOR( COLOR_PURPLE ) << "Destroying shared memory segment" << REMOVE_ALL_FORMATTING() << " - ";
        int stat = shmctl( get_shmid(), IPC_RMID,  NULL );
        if( stat != -1 ) cerr << SET_COLOR( COLOR_GREEN ) << "destroyed successfully" << REMOVE_ALL_FORMATTING() << endl;
            else cerr << SET_COLOR( COLOR_RED ) << "error occurred (" << errno << ")" << REMOVE_ALL_FORMATTING() << endl;

        /*====================  Destroy Semaphores  ====================*/
        cerr << SET_COLOR( COLOR_PURPLE ) << "Destroying semaphores" << REMOVE_ALL_FORMATTING() << endl;
        for( int semnum = 0; semnum < SEM_COUNT; semnum ++ )
        {
            cerr << SET_COLOR( COLOR_CYAN ) << "............. #" << semnum << REMOVE_ALL_FORMATTING() << " - ";
            stat = semctl( get_semid(), semnum, IPC_RMID );
            if( stat != -1 ) cerr << SET_COLOR( COLOR_GREEN ) << "destroyed successfully" << REMOVE_ALL_FORMATTING() << endl;
            else cerr << SET_COLOR( COLOR_RED ) << "error occurred (" << errno << ")" << REMOVE_ALL_FORMATTING() << endl;
        }
        semctl( SEMID, SEM_MUTEX, SETVAL, 1 );
    /*}*/
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
/*  =======================================================      OUTPUT      =======================================================  */
/*  ================================================================================================================================  */
/// @brief Function to get average of the last 4 prices of a commodity
/// @param instances deque of last 4 instances of a commodity
/// @return average price
double get_average_price(deque<double> instances)
{
    short int valCount = instances.size();
    double average = 0.0;
    for ( int i = 0 ; i < valCount ; i ++ )
    {
        double instance = instances.back();
        average += ( instance / valCount );
        instances.pop_back();
    }
    return average;
}

/// @brief Consumer function to print the commodity table with prices and avg prices
void print_commodity_table()
{
    cout << REMOVE_ALL_FORMATTING();
    cout << "+--------------+---------+----------+" << endl;
    cout << "| Currency     |  Price  | AvgPrice |" << endl;
    cout << "+--------------+---------+----------+" << endl;
    cout << SAVE_CURSOR();
    fflush( stdout );
    for(int id = 0; id <= 10; id++)
    {
        cout << "| " << COMMODITY_ID_TO_NAME[ id ] << "|" << SET_COLOR( COLOR_BLUE );
        printf( "%7.2lf ", last_instances[ id ].back() );
        cout << REMOVE_ALL_FORMATTING() << " |" << SET_COLOR( COLOR_BLUE );
        printf( "%7.2lf ", get_average_price( last_instances[ id ] ) );
        cout << REMOVE_ALL_FORMATTING() << "  |" << endl;
    }
    cout << "+--------------+---------+----------+" << endl;
}

/// @brief updates a given field in the commodity table
void update_field(int commodity_id, int column, double value, short int comp)
{
    const string ARROW_DOWN = "↓", ARROW_UP = "↑";
    NAVIGATE_CURSOR( commodity_id, column );
    
    if( comp == GREATER )
    {
        cout << SET_COLOR( COLOR_GREEN );
        printf("%7.2lf", value);
        cout << ARROW_UP << REMOVE_ALL_FORMATTING();
    }
    else if(comp == LESS)
    {
        cout << SET_COLOR( COLOR_RED );
        printf("%7.2lf", value);
        cout << ARROW_DOWN << REMOVE_ALL_FORMATTING();
    }
    else if(comp == EQUAL)
    {
        cout << SET_COLOR( COLOR_BLUE );
        printf("%7.2lf ", value);
        cout << REMOVE_ALL_FORMATTING();
    }
}

/// @brief Updates values of the given commodity in the commodity table
void update_commodity(int commodity_id, double new_price)
{
    double old_average = get_average_price( last_instances[ commodity_id ] );
    double old_price = last_instances[ commodity_id ].back();
    
    last_instances[ commodity_id ].pop_front();
    last_instances[ commodity_id ].push_back( new_price );
    
    double new_average = get_average_price( last_instances[ commodity_id ] );

    if( new_price > old_price )
        update_field( commodity_id, 2, new_price, GREATER);
    else if( new_price < old_price )
        update_field( commodity_id, 2, new_price, LESS);
    else
        update_field( commodity_id, 2, new_price, EQUAL);
    
    if( new_average > old_average )
        update_field( commodity_id, 3, new_average, GREATER);
    else if( new_average < old_average )
        update_field( commodity_id, 3, new_average, LESS);
    else
        update_field( commodity_id, 3, new_average, EQUAL);
}


/*  ================================================================================================================================  */
/*  =======================================================       MAIN       =======================================================  */
/*  ================================================================================================================================  */
int main(int argc, char *argv[])
{
    signal(SIGINT, terminate_process);
    if( ! setup(argc, argv) )
        return cerr << SET_COLOR( COLOR_RED ) << "Error occurred during resource setup" << REMOVE_ALL_FORMATTING() << endl, EXIT_FAILURE;

    short int commodity_id;
    double commodity_price;

    system( "clear" );
    print_commodity_table();
    
    while( true )
    {

        sem_wait( SEMID, SEM_FULL );
        sem_wait( SEMID, SEM_MUTEX );
        
        /* Begin consumption */
        pair<short int, double> *buf;
        auto *shm_ptr = shmat(SHMID, (void *)0, 0);
        if( shm_ptr == (void*)-1 )
        {
            sem_signal( SEMID, SEM_MUTEX );
            continue;
        }
        
        buf = (pair<short int, double> *) shm_ptr;
        Buffer buffer( buf, BUFFER_SIZE, buf[BUFFER_SIZE].first, buf[BUFFER_SIZE].second );

        if( !buffer.isempty() )
        {
            pair<short int, double> commodity_data = buffer.front();
            commodity_id = commodity_data.first;
            commodity_price = commodity_data.second;
            buffer.pop();
        
            /* Begin Processing */
            NAVIGATE_CURSOR( 12, 0 );
            cout << "\r\033[0K"  << SET_COLOR( COLOR_CYAN ) << "Latest arrival: " << REMOVE_ALL_FORMATTING() << COMMODITY_ID_TO_NAME[ commodity_id ] <<  "\b\b: $" << commodity_price;
            update_commodity( commodity_id, commodity_price );
            NAVIGATE_CURSOR( 13, 0 );
        }

        sem_signal( SEMID, SEM_MUTEX );
        sem_signal( SEMID, SEM_EMPTY );
        /* End Consumption */

        shmdt( buf ); // Detach from memory block
        /* End Processing */
    }

    return EXIT_SUCCESS;
}
