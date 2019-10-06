#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define SIZE_OF_STAT 10000000


void prec_rdtscp() {
	printf("\n\n============== clock precision measurement using rdtsc/rdtscp ===========\n");

	int i, j, counter = 0;
	uint64_t min_diff1 = 0, min_diff2 = 0, min_diff3 = 0, min_diff4 = 0, diff;
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;

	for (i = 0; i < SIZE_OF_STAT; i++) {

		asm volatile ("CPUID\n\t"
			"RDTSC\n\t"
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
			"%rax", "%rbx", "%rcx", "%rdx");

		asm volatile ("RDTSCP\n\t"
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t"
			"CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
			"%rax", "%rbx", "%rcx", "%rdx");


		diff = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 ) - ( ((uint64_t)cycles_high << 32) | cycles_low );
		if (min_diff1 == 0 || min_diff1 > diff) {
			min_diff1 = diff;
		}
	}
	printf("Reading 1 (no instructions between rdtsc/tdtscp calls): %lu clock cycles\n", min_diff1);

	for (i = 0; i < SIZE_OF_STAT; i++) {

		asm volatile ("CPUID\n\t"
			"RDTSC\n\t"
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
			"%rax", "%rbx", "%rcx", "%rdx");

		counter++;
		counter--;
		counter++;

		asm volatile ("RDTSCP\n\t"
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t"
			"CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
			"%rax", "%rbx", "%rcx", "%rdx");


		diff = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 ) - ( ((uint64_t)cycles_high << 32) | cycles_low );
		if (min_diff2 == 0 || min_diff2 > diff) {
			min_diff2 = diff;
		}
	}
	printf("Reading 2 (smallest increment): %lu clock cycles\n", min_diff2);

	printf("Precision: %f ns\n", (min_diff2 - min_diff1) / 3.2);

}

void prec_clock_gettime() {
	printf("\n\n============== clock precision measurement using clock_gettime() ===========\n");

	int i, j, counter = 0;
	uint64_t min_diff1 = 0, min_diff2 = 0, diff;
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	struct timespec start, end;

	for (i = 0; i < SIZE_OF_STAT; i++) {

		clock_gettime(CLOCK_MONOTONIC, &start);	/* mark start time */
		clock_gettime(CLOCK_MONOTONIC, &end);	/* mark start time */

		diff = 1000000000 * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	
		if(min_diff1 > diff || min_diff1 == 0)
			min_diff1 = diff;
	}
	printf("Reading 1 (no instructions between clock_gettime() calls): %lu ns\n", min_diff1);

	for (i = 0; i < SIZE_OF_STAT; i++) {

		clock_gettime(CLOCK_MONOTONIC, &start);	/* mark start time */
		counter++;
		counter--;
		clock_gettime(CLOCK_MONOTONIC, &end);	/* mark start time */

		diff = 1000000000 * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
		if (min_diff2 == 0 || min_diff2 > diff) {
			min_diff2 = diff;
		}
	}
	printf("Reading 2 (smallest increment): %lu ns\n", min_diff2);

	printf("Precision: %lu ns\n", min_diff2 - min_diff1);

}

int main(void) {

	prec_rdtscp();
	prec_clock_gettime();

	return 0;
}
