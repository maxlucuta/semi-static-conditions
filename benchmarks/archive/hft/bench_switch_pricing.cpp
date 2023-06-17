#include <benchmark/benchmark.h>
#include "option_pricing.h"
#include "../../switch.hpp"

#define SLEEP 10
#define A 100000

static int pricingFlag;
static bool threadFlag;
static double results[4];
static BranchChangerSwitch<double, const OptionPricing&> branch(
    blackScholesEuropean,
    binomialEuropean,
    batesCallOptionPrice,
    hestonCallOptionPrice
);


static OptionPricing receiveMarketData() {
    OptionPricing optionData = {0};
    generateRandomOptionInputs(optionData);
    return optionData;
}

static void runBranch() {
    do {
        pricingFlag = rand() % 4;
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (threadFlag);
}

static void runBranchless() {
    do {
        pricingFlag = rand() % 4;
        branch.setDirection(pricingFlag);
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (threadFlag);
}

static void setup(const benchmark::State& s) {
    srand(time(NULL));
    pricingFlag = rand() % 4;
    threadFlag = true;
    for (int i = 0; i < 4; i++) results[i] = rand();
}

static void benchmarkBranch(benchmark::State& s) {
    std::thread worker(runBranch);
    OptionPricing optionData = receiveMarketData();
    for (auto _ : s) {
        for (int i = 0; i < A; i++) {
            double callPrice;
            if (pricingFlag == 0) callPrice = blackScholesEuropean(optionData);
            else if (pricingFlag == 1) callPrice = binomialEuropean(optionData);
            else if (pricingFlag == 2) callPrice = batesCallOptionPrice(optionData);
            else callPrice = hestonCallOptionPrice(optionData);
            results[pricingFlag] += callPrice;
            benchmark::DoNotOptimize(results);
        }
        benchmark::ClobberMemory();
    }
    threadFlag = false;
    worker.join();
}

BENCHMARK(benchmarkBranch)->Repetitions(10)->ReportAggregatesOnly(true)->Unit(benchmark::kMillisecond);

static void benchmarkBranchless(benchmark::State& s) {
    std::thread worker(runBranchless);
    OptionPricing optionData = receiveMarketData();
    for (auto _ : s) {
        for (int i = 0; i < A; i++) {
            results[pricingFlag] += branch.branch(optionData);
            benchmark::DoNotOptimize(results);
        }
        benchmark::ClobberMemory();
    }
    threadFlag = false;
    worker.join();
}

BENCHMARK(benchmarkBranchless)->Repetitions(10)->ReportAggregatesOnly(true)->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();