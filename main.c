/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;

    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if (argc < 2)
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file> <simulator_function> <number_of_cycles>\nFor Display use 'Display <number_of_cycles>'\nFor Simulate use 'Simulate <number_of_cycles>'\nFor Single Step use 'single_step'\nFor Show Memory use 'show_mem <memory_location>'\nFunction names are case sensitive\n", argv[0]);
        exit(1);
    }

    cpu = APEX_cpu_init(argv);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }
    

    APEX_cpu_run(cpu);
    APEX_cpu_stop(cpu);
    return 0;
}