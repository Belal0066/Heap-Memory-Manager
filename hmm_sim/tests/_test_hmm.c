#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/hmm.h"

#define MAX_ALLOCS 1000000
#define getInt(x, ...) atoi(x)

extern char end, edata, etext;

void print_status(int current, int total) {
    int progress = (current * 100) / total;
    printf("\rAllocations: [%3d %%] %d/%d", progress, current, total);
    fflush(stdout); // Ensure the status is updated immediately
}

int main(int argc, char *argv[]) {
    char *ptr[MAX_ALLOCS];
    int freeStep, freeMin, freeMax, blockSize, numAllocs, j;

    // Initial program break simulation (start of your simulated heap)
    printf("etext = %p, edata=%p, end=%p, initial simulated program break=%p\n", &etext, &edata, &end, (char*)get_program_break());

    if (argc < 3 || strcmp(argv[1], "--help") == 0) {
        printf("%s num-allocs block-size [step [min [max]]]\n", argv[0]);
        exit(1);
    }

    numAllocs = getInt(argv[1]);
    if (numAllocs > MAX_ALLOCS) {
        printf("num-allocs > %d\n", MAX_ALLOCS);
        exit(1);
    }

    blockSize = getInt(argv[2]);
    freeStep = (argc > 3) ? getInt(argv[3]) : 1;
    freeMin =  (argc > 4) ? getInt(argv[4]) : 1;
    freeMax =  (argc > 5) ? getInt(argv[5]) : numAllocs;

    if (freeMax > numAllocs) {
        printf("free-max > num-allocs\n");
        exit(1);
    }

    printf("Initial simulated program break: %10p\n", get_program_break());
    printf("Allocating %d*%d bytes\n", numAllocs, blockSize);

    for (j = 0; j < numAllocs; j++) {
        ptr[j] = hmmAlloc(blockSize);

        if (ptr[j] == NULL) {
            printf("HmmAlloc returned null\n");
            exit(1);
        }

        // Print status every 10000 allocations
        if ((j + 1) % 10000 == 0) {
            print_status(j + 1, numAllocs);
        }
    }
    // Print final status
    print_status(numAllocs, numAllocs);

    printf("\nSimulated program break is now: %10p\n", get_program_break());
    printf("Freeing blocks from %d to %d in steps of %d\n", freeMin, freeMax, freeStep);

    for (j = freeMin - 1; j < freeMax; j += freeStep)
        hmmFree(ptr[j]);

    printf("After HmmFree(), simulated program break is: %10p\n", get_program_break());

    exit(EXIT_SUCCESS);
}
