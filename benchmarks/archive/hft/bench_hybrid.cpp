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

static void runBranch() 
{
    do {
        strategyFlag = !strategyFlag;
        orderFlag = !orderFlag;
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(10)
        );
    } while (threadFlag);
}

static void runBranchless() 
{
    do {
        strategyFlag = !strategyFlag;
        orderFlag = !orderFlag;
        branchStrategy.setDirection(strategyFlag);
        branchOrder.setDirection(orderFlag);
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(10)
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
	double pricingBuffer[2] = { (double)rand(), (double)rand() };
	std::thread worker(runBranch);
	OptionPricing data = receiveMarketData();
	for (auto _ : s)
	{
			if (strategyFlag) pricingBuffer[strategyFlag] = blackScholesEuropean(data);
			else pricingBuffer[strategyFlag] = binomialEuropean(data);
			if (orderFlag) NYSE.sendBuyOrder((int)pricingBuffer[orderFlag]);
			else NYSE.sendSellOrder((int)pricingBuffer[orderFlag]);
			benchmark::DoNotOptimize(pricingBuffer);
			benchmark::ClobberMemory();
	}
	threadFlag = false;
	worker.join();
}

BENCHMARK(benchmarkBranch);

static void benchmarkBranchless(benchmark::State& s)
{
	double pricingBuffer[2] = { (double)rand(), (double)rand() };
	std::thread worker(runBranchless);
	OptionPricing data = receiveMarketData();
	for (auto _ : s)
	{
		pricingBuffer[strategyFlag] = branchStrategy.branch(data);
		branchOrder.branch(NYSE, pricingBuffer[orderFlag]);
		benchmark::DoNotOptimize(pricingBuffer);
		benchmark::ClobberMemory();
		
	}
	threadFlag = false;
	worker.join();
}

BENCHMARK(benchmarkBranchless);


BENCHMARK_MAIN();

