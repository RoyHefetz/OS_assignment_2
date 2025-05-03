struct petersonlock {
    uint flag[2]; // Flag {0,1}
    uint turn; 
    uint lock_id; // Unique identifer
    uint active; // Whether this lock is active
};