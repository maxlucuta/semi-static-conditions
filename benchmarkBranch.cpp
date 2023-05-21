/* Benchmarking BranchChanger Construct against Conditional Statements

   Maximilian Lucuta
   Imperial College London
   May 2023

   Potential use case:

   The BranchChanger construct uses a 'set_direction' method to switch
   between branches by programatically altering the assembly code in a
   'branch' method to jmp to one of two functions. This is essentially
   a memcpy of 4 bytes to some area in memory and involves some sort of
   indirection, hense it would not be sensible to use this within a main
   loop and set_direction each time we evaluate a runtime condition, the
   branched code will always be more performant.

   Instead, if, for example there is a global flag that determines which
   direction of a branch should be taken, that changes periodically due
   to some runtime event, the construct may outperform a conditional 
   branch. 
*/

#include <benchmark/benchmark.h>
#include <math.h>
#include <time.h>
#include <thread>
#include "branch.h"

#define FREQ 1

using namespace std;

__attribute__((optimize("O0")))
int addOne(int i) { return i + 1; }

__attribute__((optimize("O0")))
int subOne(int i) { return i - 1; }

static bool flag;
static bool run;
static BranchChanger branch(addOne, subOne);

void changeFlagBranch()
{   
    while (run) {
        flag = !flag;
        this_thread::sleep_for(chrono::microseconds(FREQ));
    }
}

void changeFlagBranchless() 
{   
    while (run) {
        flag = !flag;
        branch.set_direction(flag);
        this_thread::sleep_for(chrono::microseconds(FREQ));
    }
}

static void doSetup(const benchmark::State& s)
{
    srand(1);
    flag = rand() & 0x1;
    run = true;
}

static void benchmarkBranch(benchmark::State& s)
{
    int* data = new int[2];
    data[0] = rand(); data[1] = rand();
    const int N = 100000;
    for (auto _ : s) {
        s.PauseTiming();
        thread worker(changeFlagBranch);
        s.ResumeTiming();
        for (int i = 0; i < N; i++) {
            if (flag) data[flag] += addOne(i);
            else data[flag] += subOne(i);
        }
        s.PauseTiming();
        run = false;
        worker.join();
        s.ResumeTiming();
        benchmark::DoNotOptimize(data);
        benchmark::ClobberMemory();
    }
    delete [] data;   
}

static void benchmarkBranchless(benchmark::State& s)
{
    int* data = new int[2];
    data[0] = rand(); data[1] = rand();
    const int N = 100000;
    for (auto _ : s) {
        s.PauseTiming();
        thread worker(changeFlagBranchless);
        s.ResumeTiming();
        for (int i = 0; i < N; i++) {
            data[flag] += branch.branch(i);
        }
        s.PauseTiming();
        run = false;
        worker.join();
        s.ResumeTiming();
        benchmark::DoNotOptimize(data);
        benchmark::ClobberMemory();
    }
    delete [] data;
}

BENCHMARK(benchmarkBranch)->Setup(doSetup);
BENCHMARK(benchmarkBranchless)->Setup(doSetup);

BENCHMARK_MAIN();