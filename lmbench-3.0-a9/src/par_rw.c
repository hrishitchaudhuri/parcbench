#include "bench.h"

int main(int ac, char** av) {
    int fd;
    char filename[256];

    /*
        lib_timing.c : contains benchmp(benchmp_f initialize, 
	                                    benchmp_f benchmark,
	                                    benchmp_f cleanup,
	                                    int enough, 
	                                    int parallel,
	                                    int warmup,
	                                    int repetitions,
	                                    void* cookie)

        benchmp_f : typedef void (*benchmp_f)(iter_t iterations, void* cookie);
            (function pointer to void fn that takes an iter_t (long) and cookie)

        initialize, benchmark, cleanup : functions obeying the benchmp_f signature
            initialize -> executed initially
            benchmark -> code that is actually benchmarked
            cleanup -> necessary destructor procedures


        enough : a param that decides how long to run the bench for.
            if 0 -> lmbench decides how long to run it
            if !0 -> timeout, unless lmbench thinks enough is too short


        parallel : like the name suggests; multiple processes execute the benchmark
                    concurrently. 

        
        warmup : how long to delay each child's run for.


        NOTE : benchmp actually executes at least one child process to run the benchmarks
            (the function is literally just a signal collector.)
    */


    /*
        Big Idea 1: 
                Benchmark function spawns multiple reader-writer processes. 
                All try to read/write to the same file. 
    */
}