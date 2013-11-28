/*
Example of how to profile using callgrind:

$ g++ -I /usr/include/valgrind main.cpp
$ valgrind --tool=callgrind --instr-atstart=no ./a.out
$ kcachegrind callgrind.out.21270 (21270 is the pid in this example)

*/

#include <cstdio>
#include "callgrind.h"

void f(int n)
{
    double x = 10;
    const double e = 1.00000123;
    for (int i = 0; i < n; ++i)
        x *= e;
    fprintf(stderr, "x (f): %lf\n", x);
}

void g(int n)
{
    double x = 10;
    const double e = 1.00000123;
    for (int i = 0; i < n; ++i)
        x *= e;
    fprintf(stderr, "x (g): %lf\n", x);
}

int main()
{
    fprintf(stderr, "bravo\n");

    CALLGRIND_START_INSTRUMENTATION;

    {
        double x = 10;
        const double e = 1.00000123;
        for (int i = 0; i < 10000; ++i)
            x *= e;
        fprintf(stderr, "x (main): %lf\n", x);
    }

    f(10000);
    g(20000);
    CALLGRIND_DUMP_STATS;
    CALLGRIND_STOP_INSTRUMENTATION;

    return 0;
}
