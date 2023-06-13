#include <benchmark/benchmark.h>
#include "../../switch.hpp"
#include <thread>

enum Strategy { 
	Price = 0, 
	Risk = 1, 
	Order = 2, 
	Buy = 3 
};

#define SLEEP 1
#define N 1000000

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a + b; }
int dib(int a, int b) { return a / b; }

static int flag;
static bool threadFlag;
static BranchChangerSwitch<int, int, int> branch(
	add, sub, mul, dib
);

static void runBranch() {
    do {
        flag = rand() % 4;
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (threadFlag);
}

static void runBranchless() {
    do {
        flag = rand() % 4;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (threadFlag);
}

static void setup(const benchmark::State& s) {
	srand(time(NULL));
	flag = rand() % 4;
	threadFlag = true;
}

static void branchCacheWarming() {
	int results[4], a = rand(), b = rand();
	for (int i = 0; i < N; i++) {
		if (flag == 0) results[flag] += add(a,b);
		else if (flag == 1) results[flag] += sub(a,b);
		else if (flag == 2) results[flag] += mul(a,b);
		else results[flag] += dib(a,b);
	}
}

static void branchlessCacheWarming() {
	int results[4], a = rand(), b = rand();
	for (int i = 0; i < N; i++) {
		results[flag] += branch.branch(a,b);
	}
}

static void benchmarkBranch(benchmark::State& s) {
	int results[4], a = rand(), b = rand();
	std::thread worker(runBranch);
	branchCacheWarming();
	for (auto _ : s) {
		if (flag == 0) results[flag] += add(a,b);
		else if (flag == 1) results[flag] += sub(a,b);
		else if (flag == 2) results[flag] += mul(a,b);
		else results[flag] += dib(a,b);
		benchmark::DoNotOptimize(results);
		benchmark::ClobberMemory();
	}
	threadFlag = false;
	worker.join();
}

BENCHMARK(benchmarkBranch);


static void benchmarkBranchless(benchmark::State& s) {
	int results[4], a = rand(), b = rand();
	std::thread worker(runBranchless);
	branchlessCacheWarming();
	for (auto _ : s) {
		results[flag] += branch.branch(a,b);
		benchmark::DoNotOptimize(results);
		benchmark::ClobberMemory();	
	}
	threadFlag = false;
	worker.join();
}


BENCHMARK(benchmarkBranchless);


BENCHMARK_MAIN();