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
#define NSEC_PER_MSEC 		(1000000)
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
/******************************************************************************************************/

/* calculates a duration in ms with added precision */
double timestamp(struct timespec *duration) {
    return (double)(duration->tv_sec) + (double)((double)duration->tv_nsec / NSEC_PER_MSEC);
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
	printf("\n***************************************************************************\n");
	printf("\tLatitude:\t%.04f\n", 	current_attitude.latitude);
	printf("\tLongitude:\t%.04f\n", current_attitude.longitude);
	printf("\tAltitude:\t%.04f\n", 	current_attitude.altitude);
	printf("\tRoll:\t\t%.04f\n", 	current_attitude.roll);
	printf("\tPitch:\t\t%.04f\n", 	current_attitude.pitch);
	printf("\tYaw:\t\t%.04f\n", 	current_attitude.yaw);
	printf("\tTimestamp:\t%.04f\n", timestamp(&current_attitude.sample_time));
	printf("***************************************************************************\n");
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
		clock_gettime(CLOCK_REALTIME, &current_attitude.sample_time);
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
		sleep_ms(READER_TIME_MS);
	}
}

int main(void) { 
	pthread_create(&update, 0, update_attitude, 0);
	pthread_create(&reader, 0, read_attitude, 0);

	sleep_ms(TOTAL_TIME_MS);
	done = true;

	pthread_join(update, NULL);
	pthread_join(reader, NULL);

	return 0;
}