/*
Programming Assignment 1 for CS 6F03 Winter 2017 Term
Filename: pa1.c
By: Omer Waseem (#000470449) and Andrew Kohnen (#001327365)
Date Submitted: Jan 29, 2017
Description: Completed as per 100% grading criteria:
	- S generated and property enforced concurrently
	- Indicates if property cannot be met with given parameters
	- Accounts for cases when M does not divide N evenly (eg N = 3, M = 5)
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

// global variables for all threads
char *S;
int len, M, L, N, i, count, start, end, flag0, flag1, flag2;
char c[3];
char alpha[] = {'a','b','c','d','e','f','g', 'h'};
pthread_mutex_t lock;

void *thread_runner (void *rank);
int checkSeg (int a, int b);
int checkProp (char val);

// main function
int main(int argc, char **argv) {
	
	int x, thread;
	pthread_t *thread_handles;
	FILE *f;
	
	// check for valid number of arguments
	if (argc != 8) {
		
		printf("Invalid input.\n");
		return 0;
		
	} else {
		
		// extract input values from arguments
		i = atoi(argv[1]);
		N = atoi(argv[2]);
		L = atoi(argv[3]);
		M = atoi(argv[4]);
		for (x = 0; x < 3; x++) {
			c[x] = argv[x + 5][0];
		}
		
		// initialize variables, handles and mutex
		S = malloc(sizeof(S) * (M * L + 1));
		S[0] = '\0'; // initialize to empty string
		len = 0;
		count = 0;
		start = 0;
		end = L - 1;
		flag0 = 0;
		flag1 = 0;
		flag2 = 0;
		thread_handles = malloc(sizeof(pthread_t) * N);
		pthread_mutex_init(&lock, NULL);
		
		// create threads
		for (thread = 0; thread < N; thread++) {
			pthread_create(&thread_handles[thread], NULL, thread_runner, (void *)thread);
		}
		
		// join threads
		for (thread = 0; thread < N; thread++) {
			pthread_join(thread_handles[thread], NULL);
		}
		
		// print string and count to screen and out.txt
		printf("\nFinal output:\n");
		printf("%s\n", S);
		printf("%d\n", count);
		
		f = fopen("out.txt", "w");
		if (f == NULL) {
			printf("Error during output to out.txt\n");
		}
		else {
			fprintf(f, "%s\n", S);
			fprintf(f, "%d\n", count);
		}
		fclose(f);
	}
	
	// free space and exit;
	free (thread_handles);
	free (S);
	pthread_mutex_destroy(&lock);
	return 0;
}

// thread function for appending characters and checking properties
// input: thread number (assigned integer)
// output: none
void *thread_runner (void *rank) {
	int thread = (int)rank;
	char val = alpha[thread];
	int r, a, b, x = 0;
	
	while (1) {
		// sleep between 100ms and 500ms
		r = (rand() % 401) + 100; // random number between 100 and 500
		struct timespec t;
		t.tv_sec = 0;
		t.tv_nsec = r * 1000000; // conversion to nano seconds
		nanosleep(&t, NULL);
		
		// mutex lock
		pthread_mutex_lock(&lock);
		
		if (len < M * L) {
			// insert new value into S and only increment if it doesn't volilate property
			S[len] = val;
			
			// CheckProp validates if val can be added to S without violated the property
			if (checkProp(val)){
				printf("Appended %c to S at index: %d, thread: %d, slept: %dms\n", val, len, thread, r);
				
				len++; // increase length count, confirming val as part of S
				
				// check if len is reached and terminate string
				if (len == M * L) {
					S[len] = '\0';
				}
			}
			else {
				// flags for tracking which c values will result in violation of property
				if (val == c[0]) {
					flag0 = 1;
				}
				else if (val == c[1]) {
					flag1 = 1;
				}
				else if (val == c[2]) {
					flag2 = 1;
				}
				
				printf("Property violation, char: %c, thread: %d not added\n", val, thread);
				
				if (N < 3) { // if N < 3 then no property can be met
					printf("String cannot be constructed with property enforced since N < 3\n");
					exit(0);
				}
				// if N == 3 and all c flags are set, a property enforced segment cannot be created
				if ((flag0 && flag1 && flag2 && N == 3) || N < 3){
					printf("String cannot be constructed with property enforced using current parameterss\n");
					exit(0);
				}
			}
		} else {
			// mutex unlock
			pthread_mutex_unlock(&lock);
			
			// checks properties on segments from 0 to floor(M/N)
			for (x = 0; x < M/N; x++) {
				a = thread * L + x * N * L;
				b = a + L - 1;
				if (b < len) {
					checkSeg(a, b);
				}
			}
			
			// checks properties on remaining segments from M/N
			if (thread < M % N) {
				a = thread * L + x * N * L;
				b = a + L - 1;
				checkSeg(a, b);
			}
			
			return NULL;
		}
		
		// mutex unlock
		pthread_mutex_unlock(&lock);
	}
	
	return NULL;
}

// function for checking segment properties
// input: a is the started index and b is the ending index for the segment
// output: flag = 0 if property is not satisfied, flag = 1 if property is satisfied
int checkSeg (int a, int b) {
	int c0 = 0, c1 = 0, c2 = 0, x = 0, flag = 0;
	
	// loop through S to determine values of c0, c1, and c2
	for (x = a; x <= b; x++) {
		if (S[x] == c[0]) {
			c0++;
		}
		else if (S[x] == c[1]) {
			c1++;
		}
		else if (S[x] == c[2]) {
			c2++;
		}
	}
	
	
	// flag to indicate property is met
	if (i == 0 && c0 + c1 == c2) {
		flag = 1;
	}
	else if (i == 1 && c0 + 2*c1 == c2) {
		flag = 1;
	}
	else if (i == 2 && c0 * c1 == c2) {
		flag = 1;
	}
	else if (i == 3 && c0 - c1 == c2) {
		flag = 1;
	}
	
	// increment count for met property using mutex
	if (flag == 1) {
		pthread_mutex_lock(&lock);
		count++;
		pthread_mutex_unlock(&lock);
	}
	
	return flag;
}

// function to check if property can be enforced
// input: new value being inserted into S
// output: flag = 0 if value cannot be inserted, flag = 1 if value can be inserted
int checkProp (char val) {
	int c0 = 0, c1 = 0, c2 = 0, x = 0, flag = 0;
	
	// free is the number of empty spaces left in the segment assuming new value is used
	int free = L - len % L - 1;
	
	// loop through S to determine values of c0, c1, and c2
	for (x = len - (len % L); x <= len; x++) {
		if (S[x] == c[0]) {
			c0++;
		}
		else if (S[x] == c[1]) {
			c1++;
		}
		else if (S[x] == c[2]) {
			c2++;
		}
	}
	
	//
	if (i == 0) {
		// if property is met reset block flags for c0, c1 and c2 and set met flag
		if (c0 + c1 == c2) {
			flag = 1;
			flag0 = 0;
			flag1 = 0;
			flag2 = 0;
		}
		else { // if property is not met, check if addition of new value leads to the property being unenforcable
			if (free == 0){
				flag = 0;
			}
			else { // determine if enough free spaces exist to accomodate future values required to satisfy property.
				if (c2 - c0 - c1 < 0 && free >= c0 + c1 - c2) {
					flag = 1;
				}
				else if (c2 - c0 - c1 > 0 && free >= c2 - c0 - c1) {
					flag = 1;
				}
				else {
					flag = 0;
				}
			}
		}
	}
	else if (i == 1) { // similar to case for i == 0
		if (c0 + 2*c1 == c2) {
			flag = 1;
			flag0 = 0;
			flag1 = 0;
			flag2 = 0;
		}
		else {
			if (free == 0){
				flag = 0;
			}
			else {
				if (c2 - c0 - c1*2 < 0 && free >= c0 + c1*2 - c2) {
					flag = 1;
				}
				else if (c2 - c0 - c1*2 > 0 && free >= c2 - c0 - c1*2) {
					flag = 1;
				}
				else {
					flag = 0;
				}
			}
		}
	}
	else if (i == 2) { // similar to case for i == 0
		if (c0 * c1 == c2) {
			flag = 1;
			flag0 = 0;
			flag1 = 0;
			flag2 = 0;
		}
		else {
			if (free == 0){
				flag = 0;
			}
			else {
				if (c2 - c0 * c1 < 0 && free >= c0 * c1 - c2) {
					flag = 1;
				}
				else if (c2 - c0 * c1 > 0 && free >= c2 - c0 * c1) {
					flag = 1;
				}
				else {
					flag = 0;
				}
			}
		}
	}
	else if (i == 3) { // similar to case for i == 0
		if (c0 - c1 == c2) {
			flag = 1;
			flag0 = 0;
			flag1 = 0;
			flag2 = 0;
		}
		else {
			if (free == 0){
				flag = 0;
			}
			else {
				if (c2 - (c0 - c1) < 0 && free >= (c0 - c1) - c2) {
					flag = 1;
				}
				else if (c2 - (c0 - c1) > 0 && free >= c2 - (c0 - c1)) {
					flag = 1;
				}
				else {
					flag = 0;
				}
			}
		}
	}
	
	return flag;
}

