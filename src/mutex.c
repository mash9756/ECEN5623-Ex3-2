/**
 *  @name   mutex
 *  @brief  simple thread-safe data manipulation example using a mutex
 * 
 *  @author Mark Sherman
 *  @date   03/9/2024
 * 
 *  @cite   https://github.com/mash9756/ECEN5623-Ex1
 *            using timestamp, delta_t, and sleep_ms previously written for the above assignment
 * 
 *	@note	currently does not work as intended, need to investigate mutex locking and timing
*/

#define _GNU_SOURCE
/* standard libraries */
#include <stdio.h>
#include <stdint.h>

/* posix thread libraries */
#include <pthread.h>
#include <semaphore.h>

/* misc libraries */
#include <time.h>
#include <signal.h>
#include <math.h>

/*  arbitrary range for update index passed to math functions*/
#define UPDATE_MAX_INDEX    (10.0000)
#define UPDATE_MIN_INDEX    (-10.0000)

/* defines for time conversions */
#define MSEC_PER_SEC  (1000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_SEC  (1000000000)

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
pthread_t read;

/* semaphores for release control */
sem_t rel_update;
sem_t rel_read;

/* global attitude resource */
attitude_t current_attitude;

/* mutex to protect access to current_attitude */
pthread_mutex_t mutex;

/* index for update value loop, using double as casting as double wasn't working for some reason */
static double i = UPDATE_MIN_INDEX;
/******************************************************************************************************/

/* calculates a duration in ms with added precision */
double timestamp(struct timespec *duration) {
    return (double)(duration->tv_sec) + (double)((double)duration->tv_nsec / NSEC_PER_MSEC);
}

int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return(1);
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
	printf("\tLatitude:\t%.04f\n", 	current_attitude.latitude);
	printf("\tLongitude:\t%.04f\n", current_attitude.longitude);
	printf("\tAltitude:\t%.04f\n", 	current_attitude.altitude);
	printf("\tRoll:\t\t%.04f\n", 	current_attitude.roll);
	printf("\tPitch:\t\t%.04f\n", 	current_attitude.pitch);
	printf("\tYaw:\t\t%.04f\n", 	current_attitude.yaw);
	printf("\tTimestamp:\t%.04f\n", timestamp(&current_attitude.sample_time));
}

void *update_attitude(void *threadp) {
	while(1) {
		sem_wait(&rel_update);
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
	}
}

void *read_attitude(void *threadp) {
	while(1){
		sem_wait(&rel_read);
	/****************************CRITICAL SECTION*****************************/
		pthread_mutex_lock(&mutex);
    	print_attitude();
		pthread_mutex_unlock(&mutex);
	/*************************************************************************/
	}
}

int main(void) { 
	struct timespec current_time;
	struct timespec prev_time;
	struct sigevent sev;
	timer_t timerid;

	pthread_create(&update, 0, update_attitude, 0);
	pthread_create(&read, 0, read_attitude, 0);

	timer_create(CLOCK_MONOTONIC, &sev, &timerid);

/* semaphores for controlling update and read service release */
	sem_init(&rel_update, 0, 1);
  	sem_init(&rel_read, 0, 0);

	while(1){
	/* get current time for comparison */
		clock_gettime(CLOCK_REALTIME, &current_time);

	/* release update every second */
		sem_post(&rel_update);

	/* release read every 100ms */
		sem_post(&rel_read);

	}
}