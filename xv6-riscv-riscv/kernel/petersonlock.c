#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "petersonlock.h"

#define NLOCKS 15

struct petersonlock peterson_locks_arr[NLOCKS];

void
init_peterson_locks_arr() {
    for(int i = 0; i < NLOCKS; i++){
        peterson_locks_arr[i].flag[0] = 0;
        peterson_locks_arr[i].flag[1] = 0;
        peterson_locks_arr[i].turn = 0;
        peterson_locks_arr[i].active = 0; // Will change once creating a lock;
        peterson_locks_arr[i].lock_id = i;
    }
}

int 
peterson_create() {
    for(int i = 0 ; i < NLOCKS; i++) {
        if(!peterson_locks_arr[i].active) {
            peterson_locks_arr[i].active = 1;
            return i;
        }
    }
    return -1;
}


int
peterson_acquire(int lock_id, int role) {
    if (lock_id < 0 || lock_id >= NLOCKS)
        return -1;
    if (role != 0 && role != 1)
        return -1;

    struct petersonlock *lock = &peterson_locks_arr[lock_id];
    int other = 1 - role; 

    if (!lock->active)
        return -1;

    lock->flag[role] = 1;
    __sync_synchronize();  // Ensure visibility

    lock->turn = role; // Set turn to the current role
    __sync_synchronize();  // Prevent reordering

    // Wait until the other is not interested or it's our turn
    while (lock->flag[other] && lock->turn == role) {
        // Let the CPU run other processes
        yield();
    }

    __sync_synchronize();  // Ensure critical section memory visibility
    // You now hold the lock

    return 0;
}


int peterson_release(int lock_id, int role) {

    if (lock_id < 0 || lock_id >= NLOCKS)
        return -1;
    if (role != 0 && role != 1)
        return -1;
    
    struct petersonlock *lock = &peterson_locks_arr[lock_id]; 

    if (!lock->active)
        return -1;
    
    __sync_synchronize();  // Ensure visibility

    __sync_lock_release(&lock->flag[role]);

    return 0;
}

int peterson_destroy(int lock_id) {
    if (lock_id < 0 || lock_id >= NLOCKS)
        return -1;

    struct petersonlock *lock = &peterson_locks_arr[lock_id];

    __sync_synchronize(); 

    // Atomically set active to 0 and check if it was already inactive
    if (__sync_lock_test_and_set(&lock->active, 0) == 0)
        return -1; // Already inactive

    __sync_synchronize(); 

    // Reset lock configurations
    lock->flag[0] = 0;
    lock->flag[1] = 0;

    return 0;
}
