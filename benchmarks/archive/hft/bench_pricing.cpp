#include <benchmark/benchmark.h>
#include "option_pricing.h"
#include "../../branch.h"


#define SLEEP 1
#define N 1000000

static bool run;
static bool flag;
static BranchChanger branch(
    blackScholesEuropean,
    binomialEuropean
);


static void runBranch() {
    do {
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (run);
}

static void runBranchless() {
    do {
        flag = !flag;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (run);
}

static OptionPricing receiveMarketData() {
    OptionPricing optionData = {0};
    generateRandomOptionInputs(optionData);
    return optionData;
}

static void branchCacheWarming() {
    double results[2] = { (double)rand(), (double)rand() };
    OptionPricing optionData = receiveMarketData();
    for (int i = 0; i < N; i++) {
        if (flag) results[flag] += blackScholesEuropean(optionData);
        else results[flag] += binomialEuropean(optionData);
    }
}

static void branchlessCacheWarming() {
    double results[2] = { (double)rand(), (double)rand() };
    OptionPricing optionData = receiveMarketData();
    for (int i = 0; i < N; i++) {
        results[flag] += branch.branch(optionData);
    }
}


static void setup(const benchmark::State& s) {
    flag = rand() % 2;
    run = true;
    srand(time(NULL));
}


static void benchmarkBranch(benchmark::State& s) {
    double results[2] = { (double)rand(), (double)rand() };
    OptionPricing optionData = receiveMarketData();
    std::thread worker(runBranch);
    branchCacheWarming();
    for (auto _ : s) {
        if (flag) results[flag] += blackScholesEuropean(optionData);
        else results[flag] += binomialEuropean(optionData);
        benchmark::DoNotOptimize(results);
        benchmark::ClobberMemory();
    } 
    run = false;
    worker.join();
}


BENCHMARK(benchmarkBranch);


static void benchmarkBranchless(benchmark::State& s) {
    double results[2] = { (double)rand(), (double)rand() };
    OptionPricing optionData = receiveMarketData();
    std::thread worker(runBranchless);
    branchlessCacheWarming();
    for (auto _ : s) {
        results[flag] += branch.branch(optionData);
        benchmark::DoNotOptimize(results);
        benchmark::ClobberMemory();
    } 
    run = false;
    worker.join();
}


BENCHMARK(benchmarkBranchless);
BENCHMARK_MAIN();





