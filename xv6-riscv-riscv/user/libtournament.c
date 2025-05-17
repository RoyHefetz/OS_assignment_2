#include "user.h"
#include "kernel/types.h"
#include "kernel/stat.h"

#define MAX_PROCESSES 16
#define MAX_LEVELS 4

// Global vars
static int *locks; // Array of locks
static int path_locks[MAX_LEVELS]; // Storing the locks IDs per "process path"
static int path_roles[MAX_LEVELS]; // Storing the roles per "process path"
static int num_processes = 0; 
static int curr_id = -1; // Current tournament ID
static int depth = 0; // Number of levels in the tour tree

// Returns the number of levels in the tree
int tour_tree_height(int n) {
    
    int levels = 0;
    while (n > 1) {
        n /= 2;
        levels++;
    }
    return levels;
}


int tournament_create(int processes) {

    if(processes > MAX_PROCESSES || processes <= 0 || (processes & (processes - 1)) != 0){
        return -1;
    }

    depth = tour_tree_height(processes);
    num_processes = processes;

    int num_locks = processes - 1;

    locks = malloc(num_locks * sizeof(int));
    if(!locks) {
        return -1;
    }

    for (int i = 0; i < num_locks; i++) {
        locks[i] = peterson_create();
        if (locks[i] < 0) {
            return -1;
        }
    }

    for(int i = 0; i < num_processes - 1; i++) {
        int pid = fork();
        if(pid < 0) {
            return -1;
        } else if(pid == 0) { // Child process
            curr_id = i + 1;
            break;
        }
    }

    if(curr_id == -1) {
        curr_id = 0; // parent process
    }

    for(int lvl = 0; lvl < depth; lvl++) {
        path_roles[lvl] = (curr_id & (1 << (depth - lvl - 1))) >> (depth - lvl - 1);
        int level_lock = curr_id >> (depth - lvl);
        int lock_index = level_lock + ((1 << lvl) - 1);
        path_locks[lvl] = locks[lock_index];

    }
        
    return curr_id;
}


int tournament_acquire(void) {

    for (int l = depth - 1; l >= 0; l--) {
        if(peterson_acquire(path_locks[l], path_roles[l]) < 0) {
            return -1;
        }
    }
    return 0;
}

int tournament_release(void) {
    
    for(int l = 0; l < depth; l++){
        if(peterson_release(path_locks[l], path_roles[l]) < 0) {
            return -1;
        }
    }
    return 0;
}
