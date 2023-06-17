#include <benchmark/benchmark.h>
#include "../utilities/utilities.hpp"
#include "../../src/branch.hpp"
#include "../../src/switch.hpp"

#define ITER 100000000
#define FREQ 0.1

/***************************** Globals ******************************/

static std::atomic<bool> flagTwo;
static std::atomic<int> flagThree;
static std::atomic<bool> thread;
static BranchChanger branchTwo(
    simple::add,
    simple::sub
);
static BranchChangerSwitch<int, int, int> branchThree(
    simple::div,
    simple::mul,
    simple::mod
);

/***************************** Threads ******************************/

static void runBranchTwo() {
    while (thread) {
        eventHappens(flagTwo);
        waitForNextEvent(FREQ);
    }
}

static void runBranchThree() {
    while (thread) {
        flagThree = std::rand() % 3;
        waitForNextEvent(FREQ);
    }
}

static void runBranchlessTwo() {
    while (thread) {
        eventHappens(flagTwo);
        branchTwo.setDirection(flagTwo);
        waitForNextEvent(FREQ);
    }
}

static void runBranchlessThree() {
    while (thread) {
        flagThree = std::rand() % 3;
        branchThree.setDirection(flagThree);
        waitForNextEvent(FREQ);
    }
}


static void setup(const benchmark::State& s) {
    flagTwo = random(0,1);
    flagThree = std::rand() % 3;
    thread = true;
}

/**************************** Branched *****************************/

static void benchmarkBranchTwo(benchmark::State& s) {
    int* results = new int[2];
    int a = random(0,N), b = random(0,N);
    std::thread worker(runBranchTwo);
    for (auto _ : s) {
        for (int i = 0; i < ITER; i++) {
            if (flagTwo) results[flagTwo] += simple::add(a,b);
            else results[flagTwo] += simple::add(a,b);
            benchmark::DoNotOptimize(results);
        }
        benchmark::ClobberMemory();
    }
    thread = false;
    worker.join();
}

static void benchmarkBranchThree(benchmark::State& s) {
    int* results = new int[3];
    int a = random(0,N), b = random(0,N);
    std::thread worker(runBranchThree);
    for (auto _ : s) {
        for (int i = 0; i < ITER; i++) {
            if (flagThree == 0) results[flagThree] += simple::div(a,b);
            else if (flagThree == 1) results[flagThree] += simple::mul(a,b);
            else results[flagThree] += simple::mod(a,b);
            benchmark::DoNotOptimize(results);
        }
        benchmark::ClobberMemory();
    }
    thread = false;
    worker.join();
}

/**************************** Branchless ***************************/

static void benchmarkBranchlessTwo(benchmark::State& s) {
    int* results = new int[2];
    int a = random(0,N), b = random(0,N);
    std::thread worker(runBranchlessTwo);
    for (auto _ : s) {
        for (int i = 0; i < ITER; i++) {
            results[flagTwo] += branchTwo.branch(a,b);
            benchmark::DoNotOptimize(results);
        }
        benchmark::ClobberMemory();
    }
    thread = false;
    worker.join();
}

static void benchmarkBranchlessThree(benchmark::State& s) {
    int* results = new int[3];
    int a = random(0,N), b = random(0,N);
    std::thread worker(runBranchlessThree);
    for (auto _ : s) {
        for (int i = 0; i < ITER; i++) {
            results[flagThree] += branchThree.branch(a,b);
            benchmark::DoNotOptimize(results);
        }
        benchmark::ClobberMemory();
    }
    thread = false;
    worker.join();
}

/***************************** Benchmark ***************************/

BENCHMARK(benchmarkBranchThree);
BENCHMARK(benchmarkBranchlessThree);
BENCHMARK_MAIN();
