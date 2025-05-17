#include "user.h"
#include "kernel/types.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: tournament <num_processes>\n");
        exit(1);
    }

    int num_processes = atoi(argv[1]);

    // Create the tournament tree and fork processes
    int tid = tournament_create(num_processes);
    if (tid < 0) {
        printf("Failed to create tournament tree\n");
        exit(1);
    }

    // Acquire the lock to enter critical section
    if (tournament_acquire() < 0) {
        printf("Process %d failed to acquire tournament lock\n", getpid());
        exit(1);
    }

    printf("Process %d (TID %d) entered critical section\n", getpid(), tid);
    sleep(1);
    printf("Process %d (TID %d) exiting critical section\n", getpid(), tid);
  
    // Release the lock
    if (tournament_release() < 0) {
        printf("Process %d failed to release tournament lock\n", getpid());
        exit(1);
    }

    exit(0);
}
