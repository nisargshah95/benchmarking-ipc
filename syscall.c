#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE_OF_STAT 10000000


int main(void) {
	printf("\n\n============== getpid() syscall time measurement ===========\n");
	int i, j;
	uint64_t min_diff1 = 0, min_diff2 = 0, diff;

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

		getpid();

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
	printf("Reading 2 (invoke getpid system call): %lu clock cycles\n", min_diff2);

	printf("Elapsed time: %.4f ns\n", (min_diff2 - min_diff1) / 3.2);


	for (i = 0; i < SIZE_OF_STAT; i++) {

		asm volatile ("CPUID\n\t"
			"RDTSC\n\t"
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
			"%rax", "%rbx", "%rcx", "%rdx");

		getuid();

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
	printf("Reading 3 (invoke getuid system call): %lu clock cycles\n", min_diff2);

	printf("Elapsed time: %.4f ns\n", (min_diff2 - min_diff1) / 3.2);

	return 0;
}
