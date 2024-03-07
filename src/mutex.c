/**
 *  @name   mutex
 *  @brief  simple thread-safe data manipulation example using a mutex
 * 
 *  @author Mark Sherman
 *  @date   03/9/2024
 * 
 *  @cite   https://github.com/mash9756/ECEN5623-Ex1
 *            using timestamp, and sleep_ms previously written for the above assignment
*/

#define _GNU_SOURCE
/* standard libraries */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* posix thread libraries */
#include <pthread.h>

/* misc libraries */
#include <time.h>
#include <math.h>

/* arbitrary range for update index passed to math functions */
#define UPDATE_MAX_INDEX    (10.0000)
#define UPDATE_MIN_INDEX    (-10.0000)

/* time conversion values */
#define MSEC_PER_SEC  		(1000)
#define MSEC_PER_SEC_D  	(1000.00)
#define NSEC_PER_MSEC 		(1000000.00)
#define NSEC_PER_SEC  		(1000000000)

/* thread timing values */
#define UPDATE_TIME_MS		(1000)
#define READER_TIME_MS		(10000)
#define TOTAL_TIME_MS		(180000)

/* attitude structure definition */
typedef struct
{
    double latitude;
    double longitude;
    double altitude;
    double roll;
    double pitch;
    double yaw;
    struct timespec sample_time;
} attitude_t;

/*	Globals	*/
/******************************************************************************************************/
/* thread IDs */
pthread_t update;
pthread_t reader;

/* global attitude resource */
attitude_t current_attitude;

/* mutex to protect access to current_attitude */
pthread_mutex_t mutex;

/* thread done flag */
bool done = false;

struct timespec start_time;
/******************************************************************************************************/

/* calculates a timestamp in ms with added precision */
double timestamp_ms(struct timespec *start, struct timespec *stop) {
	double start_sec 	= (double)(start->tv_sec);
	double start_nsec 	= (double)(start->tv_nsec);
	double stop_sec 	= (double)(stop->tv_sec);
	double stop_nsec 	= (double)(stop->tv_nsec);

	double start_ms		= (start_sec * MSEC_PER_SEC_D) + (start_nsec / NSEC_PER_MSEC);
	double stop_ms		= (stop_sec * MSEC_PER_SEC_D) + (stop_nsec / NSEC_PER_MSEC);

	return (stop_ms - start_ms);
}

/* calculates a timestamp in s with added precision */
double timestamp_s(struct timespec *start, struct timespec *stop) {
	double start_sec 	= (double)(start->tv_sec);
	double start_nsec 	= (double)(start->tv_nsec);
	double stop_sec 	= (double)(stop->tv_sec);
	double stop_nsec 	= (double)(stop->tv_nsec);

	double start_s		= (start_sec) + (start_nsec / NSEC_PER_SEC);
	double stop_s		= (stop_sec) + (stop_nsec / NSEC_PER_SEC);

	return (stop_s - start_s);
}

/* calculate total current time in seconds and print */
void get_total_time(){
	struct timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	printf("Total Time: %.02fs\n", timestamp_s(&start_time, &current_time));
}

/* simple sleep function with ms parameter for ease of use */
int sleep_ms(int ms) {
    struct timespec req_time;
    struct timespec rem_time;

    int sec   = ms / MSEC_PER_SEC;
    int nsec  = (ms % MSEC_PER_SEC) * NSEC_PER_MSEC;

    req_time.tv_sec   = sec;
    req_time.tv_nsec  = nsec;

    if(nanosleep(&req_time , &rem_time) < 0) {
        printf("Nano sleep system call failed \n");
        return -1;
    }
}

void print_attitude(void){
	printf("\n*****************************************************************\n");
	printf("\tLatitude:\t%.04f\n", 		current_attitude.latitude);
	printf("\tLongitude:\t%.04f\n", 	current_attitude.longitude);
	printf("\tAltitude:\t%.04f\n", 		current_attitude.altitude);
	printf("\tRoll:\t\t%.04f\n", 		current_attitude.roll);
	printf("\tPitch:\t\t%.04f\n", 		current_attitude.pitch);
	printf("\tYaw:\t\t%.04f\n", 		current_attitude.yaw);
	printf("\tTimestamp:\t%.02fms\n", 	timestamp_ms(&start_time, &current_attitude.sample_time));
	printf("*****************************************************************\n");
}

void *update_attitude(void *threadp) {
/* index for update value loop, using double for type compat with math functions */
	double i = UPDATE_MIN_INDEX;
	while(!done) {
	/****************************CRITICAL SECTION*****************************/
		pthread_mutex_lock(&mutex);
		current_attitude.latitude       = (0.01 * i);
		current_attitude.longitude      = (0.2  * i);
		current_attitude.altitude       = (0.25 * i);
		current_attitude.roll           = sin(i);
		current_attitude.pitch          = cos(i * i);
		current_attitude.yaw            = cos(i);
		clock_gettime(CLOCK_MONOTONIC, &current_attitude.sample_time);
		pthread_mutex_unlock(&mutex);
	/*************************************************************************/
	/* increment update index, reset to min value if necessary*/
		i++;
		if(i >= UPDATE_MAX_INDEX) {
			i = UPDATE_MIN_INDEX;
		}
		sleep_ms(UPDATE_TIME_MS);
	}
}

void *read_attitude(void *threadp) {
	int cnt = 0;
	while(!done){
		printf("\nRead Count: %d", cnt++);
	/****************************CRITICAL SECTION*****************************/
		pthread_mutex_lock(&mutex);
    	print_attitude();
		pthread_mutex_unlock(&mutex);
	/*************************************************************************/
		get_total_time();
		sleep_ms(READER_TIME_MS);
	}
}

int main(void) { 
/* save exact start time, could assume 0 here but we want best accuracy */
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	pthread_create(&update, 0, update_attitude, 0);
	pthread_create(&reader, 0, read_attitude, 0);

/* sleep main so reader/update thread can run, set done flag to end */
	sleep_ms(TOTAL_TIME_MS);
	done = true;

	pthread_join(update, NULL);
	pthread_join(reader, NULL);

	get_total_time();
	return 0;
}