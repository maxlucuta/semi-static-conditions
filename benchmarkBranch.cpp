/* Benchmarking BranchChanger Construct against Conditional Statements

   Maximilian Lucuta
   Imperial College London
   May 2023
*/

#include <benchmark/benchmark.h>
#include <math.h>
#include <time.h>
#include <thread>
#include "branch.h"

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
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void changeFlagBranchless()
{   
    while (run) {
        flag = !flag;
        branch.set_direction(flag);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
        std::thread worker(changeFlagBranch);
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
        std::thread worker(changeFlagBranchless);
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