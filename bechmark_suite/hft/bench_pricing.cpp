#include <benchmark/benchmark.h>
#include "option_pricing.h"
#include "../../branch.h"


#define N 1000000


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
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(1000)
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
        flag = !flag;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::microseconds(1000)
        );
    } while (run);
}

static void setup(const benchmark::State& s) {
    flag = rand() % 2;
    run = true;
    srand(time(NULL));
}


static void benchmarkBranch(benchmark::State& s) {
    int sleep = s.range(0);
    std::vector<OptionPricing> optionData;
    optionData.reserve(N);
    double existingData[2] = {(double)rand(), (double)rand()};
    for (int i = 0; i < N; i++) {
        OptionPricing priceData = {0};
        generateRandomOptionInputs(priceData);
        optionData.push_back(priceData);
    }
    std::thread worker(runBranch, sleep);
    for (auto _ : s) {
        for (auto option : optionData) {
            if (flag) option.callPrice = blackScholesEuropean(option);
            else option.callPrice = binomialEuropean(option);
            existingData[flag] += option.callPrice;
            benchmark::DoNotOptimize(optionData);
            benchmark::DoNotOptimize(existingData);
        }
        benchmark::ClobberMemory();
    }
    run = false;
    worker.join();
}


BENCHMARK(benchmarkBranch)
    ->DenseRange(1,10)
    ->DenseRange(20,100,10)
    //->DenseRange(200,1000,100)
    //->DenseRange(2000,10000,1000)
    //->DenseRange(20000,100000,10000)
    //->Setup(setup)
    //->Repetitions(10)
    //->ReportAggregatesOnly(true)
    ->MinWarmUpTime(N)
    ->Unit(benchmark::kMillisecond);


static void benchmarkBranchless(benchmark::State& s) {
    int sleep = s.range(0);
    std::vector<OptionPricing> optionData;
    optionData.reserve(N);
    double existingData[2] = {(double)rand(), (double)rand()};
    for (int i = 0; i < N; i++) {
        OptionPricing priceData = {0};
        generateRandomOptionInputs(priceData);
        optionData.push_back(priceData);
    }
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (auto option : optionData) {
            option.callPrice = branch.branch(option);
            existingData[flag] += option.callPrice;
            benchmark::DoNotOptimize(optionData);
            benchmark::DoNotOptimize(existingData);
        }
        benchmark::ClobberMemory();
    }
    run = false;
    worker.join();
}


BENCHMARK(benchmarkBranchless)
    ->DenseRange(1,10)
    ->DenseRange(20,100,10)
    //->DenseRange(200,1000,100)
    //->DenseRange(2000,10000,1000)
    //->DenseRange(20000,100000,10000)
    //->Setup(setup)
    //->Repetitions(10)
    //->ReportAggregatesOnly(true)
    ->MinWarmUpTime(N)
    ->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();





