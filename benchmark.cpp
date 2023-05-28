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
#include <thread>
#include "branch.h"


#define N 20000000

int memory[2] = { rand(), rand() };

static void foo() { memory[0] = rand(); }
static void bar() { memory[1] = rand(); }

static bool flag = true;
static bool run = true;
static BranchChanger branch(foo, bar);


static void runBranchless(int sleep) {
    do {
        flag = !flag;
        branch.set_direction(flag);
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (run);
}

static void runBranch(int sleep) {
    do {
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (run);
}

static void setup(const benchmark::State& s) { 
    flag = run = true;
    memory[0] = rand();
    memory[1] = rand();
}

static void benchmarkBranch(benchmark::State& s) {
    int sleep = s.range(0);
    int data[2] = { rand(), rand() };
    int a = rand(), b = rand();
    std::thread worker(runBranch, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            if (flag) foo();
            else bar();
            data[flag] += memory[flag];
            benchmark::DoNotOptimize(data);
            benchmark::DoNotOptimize(memory);
        }
        benchmark::ClobberMemory();
    }
    run = false;
    worker.join();
}

BENCHMARK(benchmarkBranch)
    ->DenseRange(1,10)
    ->DenseRange(20,100,10)
    ->DenseRange(200,1000,100)
    ->DenseRange(2000,10000,1000)
    ->DenseRange(20000,100000,10000)
    ->Setup(setup);

static void benchmarkBranchless(benchmark::State& s) {
    int sleep = s.range(0);
    int data[2] = { rand(), rand() };
    int a = rand(), b = rand();
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            branch.branch();
            data[flag] += memory[flag];
            benchmark::DoNotOptimize(data);
            benchmark::DoNotOptimize(memory);
        }
        benchmark::ClobberMemory();
    }
    run = false;
    worker.join();
}

BENCHMARK(benchmarkBranchless)
    ->DenseRange(1,10)
    ->DenseRange(20,100,10)
    ->DenseRange(200,1000,100)
    ->DenseRange(2000,10000,1000)
    ->DenseRange(20000,100000,10000)
    ->Setup(setup);


BENCHMARK_MAIN();



