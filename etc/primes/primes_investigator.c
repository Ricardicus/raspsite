#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

int nbr_of_ones(uint64_t nbr){
	int i = 0;
	int count = 0;

	for (; i < 8*8 ; ++i ){

		if ( nbr >> i & 1 ) {
			++count;
		}

	}

	return count;
}

int main (int argc, char *argv[]) {
	FILE *fp_r, *fp_w;
	char primes_buffer[20];

	if ( argc < 2 ) {
		printf("Usage: %s limit.\n", argv[0]);
		return -1;
	}

	int limit = atoi(argv[1]), max = 0, ones = 0;

	fp_r = fopen("primes.txt", "r"), fp_w = fopen("primes_sieved.txt", "w");

	if ( fp_r == NULL || fp_w == NULL ) {
		printf("Error, file could not be opened!\n");
		return -1;
	}

	memset(primes_buffer, '\0', sizeof primes_buffer);


	while ( fgets(primes_buffer, 19, fp_r) != NULL ) {
		uint64_t prime = atoll(primes_buffer);
		ones = nbr_of_ones(prime);

		if (ones > max) {
			max = ones;
		}

		printf("Prime: %llu has %d ones [max: %d ones].\n", prime, ones, max);

		if ( ones <= limit ){
			fprintf(fp_w, "%llu\n", prime);
		}

	}

	fclose(fp_w), fclose(fp_r);

	return 0;
}