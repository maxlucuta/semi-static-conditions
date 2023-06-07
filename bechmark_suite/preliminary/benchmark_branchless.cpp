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

#include </benchmark/benchmark.h>
#include <thread>
#include <mutex>
#include "../../branch.h"

#define N 100000000


static int someWork() { return rand(); }
static int someOtherWork() { return -rand(); }

static int someWork_1(int a) { return a + rand(); }
static int someOtherWork_1(int a) { return a - rand(); }

static int someWork_2 (int a, int b) { return a + b + rand(); }
static int someOtherWork_2 (int a, int b) { return a - b - rand(); }

std::mutex flag_lock;
static bool flag = true;
static bool run = true;

static BranchChanger branch1(someWork, someOtherWork);
static BranchChanger branch2(someWork_1, someOtherWork_1);
static BranchChanger branch3(someWork_2, someOtherWork_2);


static void runBranchless(int sleep) {
    do {
        flag = !flag;
        flag_lock.lock();
        branch1.set_direction(flag);
        flag_lock.unlock();
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (run);
}

static void setup(const benchmark::State& s) {
    flag = true;
    run = true;
    srand(time(NULL));
}

static void benchmarkBranchless1(benchmark::State& s) {
    int sleep = s.range(0);
    int data[2] = { rand(), rand() };
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            data[flag] += branch1.branch();
            benchmark::DoNotOptimize(data);
        }
        benchmark::ClobberMemory();
    }
    run = false;
    worker.join();
}

static void benchmarkBranchless2(benchmark::State& s) {
    int sleep = s.range(0);
    int data[2] = { rand(), rand() };
    int a = rand();
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            data[flag] += branch2.branch(a);
            benchmark::DoNotOptimize(data);
        }
        benchmark::ClobberMemory();
    }
    run = false;
    worker.join();
}

static void benchmarkBranchless3(benchmark::State& s) {
    int sleep = s.range(0);
    int data[2] = { rand(), rand() };
    int a = rand(), b = rand();
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            data[flag] += branch3.branch(a, b);
            benchmark::DoNotOptimize(data);
        }
        benchmark::ClobberMemory();
    }
    run = false;
    worker.join();
}

static void benchmarkBranchless4(benchmark::State& s) {
    int sleep = s.range(0);
    int data[2] = { rand(), rand() };
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            flag_lock.lock();
            data[flag] += branch1.branch();
            flag_lock.unlock();
            benchmark::DoNotOptimize(data);
        }
        benchmark::ClobberMemory();
    }
    run = false;
    worker.join();
}


BENCHMARK(benchmarkBranchless4)
    ->DenseRange(1,10)
    ->DenseRange(20,100,10)
    ->DenseRange(200,1000,100)
    ->DenseRange(2000,10000,1000)
    ->DenseRange(20000,100000,10000)
    ->Setup(setup)
    ->Repetitions(10)
    ->ReportAggregatesOnly(true)
    ->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();