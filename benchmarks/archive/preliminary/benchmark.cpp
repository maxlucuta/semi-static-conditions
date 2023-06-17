#include <benchmark/benchmark.h>
#include <thread>
#include "../../branch.h"

#define SLEEP 2

static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static BranchChanger branch(add, sub);
static bool run;
static bool flag;


static void runBranch() {
    do {
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(SLEEP)
        );
    } while (run);
}

static void runBranchless() {
    do {
        flag = !flag;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::microseconds(SLEEP)
        );
    } while (run);
}

static void setup(const benchmark::State& s) {
	srand(time(NULL));
	flag = rand() % 2;
	run = true;
}

static void benchmarkBranch(benchmark::State& s) {
	int results[2] = { rand(), rand() };
	int a = rand(), b = rand();
	std::thread worker(runBranch);
	for (auto _ : s) {
		if (flag) results[flag] += add(a, b);
		else results[flag] += sub(a, b);
		benchmark::DoNotOptimize(results);
		benchmark::ClobberMemory();
	}
	run = false;
	worker.join();
}

static void benchmarkBranchless(benchmark::State& s) {
	int results[2] = { rand(), rand() };
	int a = rand(), b = rand();
	std::thread worker(runBranchless);
	for (auto _ : s) {
		results[flag] = branch.branch(a, b);
		benchmark::DoNotOptimize(results);
		benchmark::ClobberMemory();
	}
	run = false;
	worker.join();
}

BENCHMARK(benchmarkBranch);
BENCHMARK(benchmarkBranchless);
BENCHMARK_MAIN();