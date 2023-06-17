#include <benchmark/benchmark.h>
#include "dummy_exchange.h"
#include "../../branch.h"

#define SLEEP 1
#define N 100000

static Exchange NYSE(7675, "99.82.232.1");
static Exchange NASDAQ(1776, "34.32.198.16");
static Exchange routes[2] = {NYSE, NASDAQ};
static bool orderFlag;
static bool threadFlag;
static BranchChangerClass branchOrder(
	&Exchange::sendBuyOrder,
	&Exchange::sendSellOrder
);

static void branchCacheWarming() {
	int amount = rand();
	for (int i = 0; i < N; i++) {
		if (orderFlag) routes[orderFlag].sendBuyOrder(amount);
		else routes[orderFlag].sendSellOrder(amount);
		
	}
}

static void branchlessCacheWarming() {
	int amount = rand();
	for (int i = 0; i < N; i++) {
		branchOrder.branch(routes[orderFlag], amount);
		
	}
}


static void runBranch() 
{
    do {
        orderFlag = !orderFlag;
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (threadFlag);
}

static void runBranchless() 
{
    do {
        orderFlag = !orderFlag;
        branchOrder.setDirection(orderFlag);
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (threadFlag);
}

static void setup(const benchmark::State& s)
{
	srand(time(NULL));
	orderFlag = rand() % 2;
	threadFlag = true;
}

static void benchmarkBranch(benchmark::State& s)
{	
	int amount = rand();
	std::thread worker(runBranch);
	branchCacheWarming();
	for (auto _ : s)
	{
		if (orderFlag) routes[orderFlag].sendBuyOrder(amount);
		else routes[orderFlag].sendSellOrder(amount);
		
	}
	threadFlag = false;
	worker.join();
}

static void benchmarkBranchless(benchmark::State& s)
{
	int amount = rand();
	std::thread worker(runBranchless);
	branchlessCacheWarming();
	for (auto _ : s)
	{
		branchOrder.branch(routes[orderFlag], amount);
		
	}
	threadFlag = false;
	worker.join();
}

BENCHMARK(benchmarkBranch);
BENCHMARK(benchmarkBranchless);
BENCHMARK_MAIN();