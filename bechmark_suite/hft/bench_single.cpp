#include <benchmark/benchmark.h>
#include "option_pricing.h"
#include "../../branch.h"

#define CACHE_WARMING 1000000

static bool run;
static bool flag;
static BranchChanger branch(
    blackScholesEuropean,
    binomialEuropean
);

static OptionPricing receiveMarketData() {
	OptionPricing optionData = {0};
	generateRandomOptionInputs(optionData);
	return optionData;
}

static void branchCacheWarming() {
	double callPrice = rand();
	OptionPricing optionData = receiveMarketData();
	for (int i = 0; i < CACHE_WARMING; i++) {
		if (flag) callPrice += blackScholesEuropean(optionData);
		else callPrice -= binomialEuropean(optionData);
	}
}

static void branchlessCacheWarming() {
	double callPrice = rand();
	OptionPricing optionData = receiveMarketData();
	for (int i = 0; i < CACHE_WARMING; i++) {
		callPrice += branch.branch(optionData);
		benchmark::DoNotOptimize(callPrice);
	}
}

static void runBranch(int sleep) {
    do {
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(rand() % sleep)
        );
    } while (run);
}

static void runBranchless(int sleep) {
    do {
        flag = !flag;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(rand() % sleep)
        );
    } while (run);
}

static void runControl(int sleep) {
	do {
        flag = flag;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(rand() % sleep)
        );
    } while (run);
}

static void setup(const benchmark::State& s) {
	srand(time(NULL));
    flag = rand() % 2;
    run = true;
}

static void benchmarkControl(benchmark::State& s) {
	int sleep = s.range(0);
	double callPrice = rand();
	OptionPricing optionData = receiveMarketData();
	std::thread worker(runBranch, sleep);
	for (auto _ : s) {
		if (true) callPrice += blackScholesEuropean(optionData);
		else callPrice -= binomialEuropean(optionData);
		benchmark::DoNotOptimize(callPrice);
		benchmark::ClobberMemory();
	}
	run = false;
	worker.join();
}

BENCHMARK(benchmarkControl)->DenseRange(1,10)->Setup(setup);

static void benchmarkBranch(benchmark::State& s) {
	int sleep = s.range(0);
	double callPrice = rand();
	OptionPricing optionData = receiveMarketData();
	std::thread worker(runBranch, sleep);
	branchlessCacheWarming();
	for (auto _ : s) {
		if (flag) callPrice += blackScholesEuropean(optionData);
		else callPrice -= binomialEuropean(optionData);
		benchmark::DoNotOptimize(callPrice);
		benchmark::ClobberMemory();
	}
	run = false;
	worker.join();
}

BENCHMARK(benchmarkBranch)->DenseRange(1,10)->Setup(setup);

static void benchmarkBranchless(benchmark::State& s) {
	int sleep = s.range(0);
	double callPrice = rand();
	OptionPricing optionData = receiveMarketData();
	std::thread worker(runBranchless, sleep);
	branchlessCacheWarming();
	for (auto _ : s) {
		callPrice += branch.branch(optionData);
		benchmark::DoNotOptimize(callPrice);
		benchmark::ClobberMemory();
	}
	run = false;
	worker.join();
}

BENCHMARK(benchmarkBranchless)->DenseRange(1,10)->Setup(setup);
BENCHMARK_MAIN();