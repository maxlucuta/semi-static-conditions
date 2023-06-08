#include <benchmark/benchmark.h>
#include "option_pricing.h"
#include "../../branch.h"


#define N 100000


static bool run = true;
static bool flag = true;
static BranchChanger branch(
    blackScholesEuropean,
    binomialEuropean
);


static void runBranch(int sleep) {
    do {
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (run);
}

static void runBranchless(int sleep) {
    do {
        flag = !flag;
        branch.setDirection(flag);
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


static void benchmarkBranch(benchmark::State& s) {
    int sleep = s.range(0);
    OptionPricing optionData[N];
    double callPrices[2];
    for (int i = 0; i < N; i++) {
    	OptionPricing priceData = {0};
    	generateRandomOptionInputs(priceData);
    	optionData[i] = priceData;
    }
    std::thread worker(runBranch, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
        	if (flag) callPrices[flag] = blackScholesEuropean(optionData[i]);
        	else callPrices[flag] = binomialEuropean(optionData[i]);
        	benchmark::DoNotOptimize(callPrices);
        	benchmark::DoNotOptimize(optionData);
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
    ->Setup(setup)
    ->Repetitions(10)
    ->ReportAggregatesOnly(true)
    ->Unit(benchmark::kMillisecond);


static void benchmarkBranchless(benchmark::State& s) {
    int sleep = s.range(0);
    OptionPricing optionData[N];
    double callPrices[2];
    for (int i = 0; i < N; i++) {
    	OptionPricing priceData = {0};
    	generateRandomOptionInputs(priceData);
    	optionData[i] = priceData;
    }
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            callPrices[flag] = branch.branch(optionData[i]);
            benchmark::DoNotOptimize(callPrices);
            benchmark::DoNotOptimize(optionData);
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
    ->Setup(setup)
    ->Repetitions(10)
    ->ReportAggregatesOnly(true)
    ->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();





