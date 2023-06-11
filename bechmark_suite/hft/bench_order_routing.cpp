#include <benchmark/benchmark.h>
#include "dummy_exchange.h"
#include "../../branch.h"

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

static void runBranch(int sleep) 
{
    do {
        orderFlag = !orderFlag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (threadFlag);
}

static void runBranchless(int sleep) 
{
    do {
        orderFlag = !orderFlag;
        branchOrder.setDirection(orderFlag);
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
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
	int sleep = s.range(0);
	std::thread worker(runBranch, sleep);
	for (auto _ : s)
	{
		for (int i = 0; i < N; i++)
		{
			if (orderFlag) routes[orderFlag].sendBuyOrder(rand());
			else routes[orderFlag].sendSellOrder(rand());
		}
	}
	threadFlag = false;
	worker.join();
}

static void benchmarkBranchless(benchmark::State& s)
{
	int sleep = s.range(0);
	std::thread worker(runBranchless, sleep);
	for (auto _ : s)
	{
		for (int i = 0; i < N; i++)
		{
			branchOrder.branch(routes[orderFlag], rand());
		}
	}
	threadFlag = false;
	worker.join();
}



BENCHMARK(benchmarkBranch)->DenseRange(1,10);
BENCHMARK(benchmarkBranchless)->DenseRange(1,10);
BENCHMARK_MAIN();