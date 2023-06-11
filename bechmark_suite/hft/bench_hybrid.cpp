#include <benchmark/benchmark.h>
#include <time.h>
#include "dummy_exchange.h"
#include "option_pricing.h"
#include "../../branch.h"

#define N 100000

static Exchange NYSE;
static bool strategyFlag;
static bool orderFlag;
static bool threadFlag;
static BranchChanger branchStrategy(
	blackScholesEuropean,
	binomialEuropean
);
static BranchChangerClass branchOrder(
	&Exchange::sendBuyOrder,
	&Exchange::sendSellOrder
);

static OptionPricing receiveMarketData()
{
	OptionPricing optionData = {0};
	generateRandomOptionInputs(optionData);
	return optionData;
}

static void runBranch(int sleep) 
{
    do {
        strategyFlag = !strategyFlag;
        orderFlag = !orderFlag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (threadFlag);
}

static void runBranchless(int sleep) 
{
    do {
        strategyFlag = !strategyFlag;
        orderFlag = !orderFlag;
        branchStrategy.setDirection(strategyFlag);
        branchOrder.setDirection(orderFlag);
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (threadFlag);
}

static void setup(const benchmark::State& s)
{
	srand(time(NULL));
	strategyFlag = rand() % 2;
	orderFlag = rand() % 2;
	threadFlag = true;
}

static void benchmarkBranch(benchmark::State& s)
{
	int sleep = s.range(0);
	double pricingBuffer[2];
	std::thread worker(runBranch, sleep);
	for (auto _ : s)
	{
		for (int i = 0; i < N; i++)
		{
			OptionPricing data = receiveMarketData();
			if (strategyFlag) pricingBuffer[strategyFlag] = blackScholesEuropean(data);
			else pricingBuffer[strategyFlag] = binomialEuropean(data);
			if (orderFlag) NYSE.sendBuyOrder((int)pricingBuffer[orderFlag]);
			else NYSE.sendSellOrder((int)pricingBuffer[orderFlag]);
			benchmark::DoNotOptimize(pricingBuffer);
			benchmark::ClobberMemory();
		}
	}
	threadFlag = false;
	worker.join();
	auto result = NYSE.getPendingOrders();
	benchmark::DoNotOptimize(result);
}

BENCHMARK(benchmarkBranch)
    ->DenseRange(1,10)
    /*
    ->DenseRange(20,100,10)
    ->DenseRange(200,1000,100)
    ->DenseRange(2000,10000,1000)
    ->DenseRange(20000,100000,10000)
    ->Setup(setup)
    ->Repetitions(10)
    ->ReportAggregatesOnly(true)
    ->MinWarmUpTime(N)
    */
    ->Unit(benchmark::kMillisecond);

static void benchmarkBranchless(benchmark::State& s)
{
	int sleep = s.range(0);
	double pricingBuffer[2];
	std::thread worker(runBranchless, sleep);
	for (auto _ : s)
	{
		for (int i = 0; i < N; i++)
		{
			OptionPricing data = receiveMarketData();
			pricingBuffer[strategyFlag] = branchStrategy.branch(data);
			branchOrder.branch(NYSE, pricingBuffer[orderFlag]);
			benchmark::DoNotOptimize(pricingBuffer);
			benchmark::ClobberMemory();
		}
	}
	threadFlag = false;
	worker.join();
	auto result = NYSE.getPendingOrders();
	benchmark::DoNotOptimize(result);
}

BENCHMARK(benchmarkBranchless)
    ->DenseRange(1,10)
    /*
    ->DenseRange(20,100,10)
    ->DenseRange(200,1000,100)
    ->DenseRange(2000,10000,1000)
    ->DenseRange(20000,100000,10000)
    ->Setup(setup)
    ->Repetitions(10)
    ->ReportAggregatesOnly(true)
    ->MinWarmUpTime(N)
    */
    ->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();

