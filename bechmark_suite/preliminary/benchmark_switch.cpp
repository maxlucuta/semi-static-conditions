#include <benchmark/benchmark.h>
#include "../../switch.hpp"
#include <thread>
#include <math.h>

#define SLEEP 1
#define A 1000000

__attribute__((noinline))
int add(int a, int b) { return a + b; }
__attribute__((noinline))
int sub(int a, int b) { return a - b; }
__attribute__((noinline))
int mul(int a, int b) { return a + b; }
__attribute__((noinline))
int dib(int a, int b) { return a / b; }
__attribute__((noinline))
int mod(int a, int b) { return a % b; }

static int flag;
static bool threadFlag;
static BranchChangerSwitch<int, int, int> branch(
	add, sub, mul, dib, mod
);

static void runBranch() {
    do {
        flag = rand() % 5;
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (threadFlag);
}

static void runBranchless() {
    do {
        flag = rand() % 5;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::nanoseconds(SLEEP)
        );
    } while (threadFlag);
}

static void setup(const benchmark::State& s) {
	srand(time(NULL));
	flag = rand() % 5;
	threadFlag = true;
}

static void branchCacheWarming() {
	int results[5] = { rand(), rand(), rand(), rand(), rand() };
	int a = rand(), b = rand();
	for (int i = 0; i < A; i++) {
		
		switch (flag) {
			case 0:
				results[flag] += add(a,b);
				break;
			case 1:
				results[flag] += sub(a,b);
				break;
			case 2:
				results[flag] += mul(a,b);
				break;
			case 3:
				results[flag] += dib(a,b);
				break;
			default:
				results[flag] += mod(a,b);
				break;
		}
		/*
			if (flag == 0) results[flag] += add(a,b);
			else if (flag == 1) results[flag] += sub(a,b);
			else if (flag == 2) results[flag] += mul(a,b);
			else if (flag == 3) results[flag] += dib(a,b);
			else results[flag] += mod(a,b);
			*/
	}
}

static void branchlessCacheWarming() {
	int results[5] = { rand(), rand(), rand(), rand(), rand() };
	int a = rand(), b = rand();
	for (int i = 0; i < A; i++) {
		results[flag] += branch.branch(a,b);
	}
}

static void benchmarkBranch(benchmark::State& s) {
	int results[5] = { rand(), rand(), rand(), rand(), rand() };
	int a = rand(), b = rand();
	branchCacheWarming();
	std::thread worker(runBranch);
	for (auto _ : s) {
		for (int i = 0; i < A; i++) {
			switch (flag) {
			case 0:
				results[flag] += add(a,b);
				break;
			case 1:
				results[flag] += sub(a,b);
				break;
			case 2:
				results[flag] += mul(a,b);
				break;
			case 3:
				results[flag] += dib(a,b);
				break;
			default:
				results[flag] += mod(a,b);
				break;
			}
			/*
			if (flag == 0) results[flag] += add(a,b);
			else if (flag == 1) results[flag] += sub(a,b);
			else if (flag == 2) results[flag] += mul(a,b);
			else if (flag == 3) results[flag] += dib(a,b);
			else results[flag] += mod(a,b);
			*/
			benchmark::DoNotOptimize(results);
			benchmark::ClobberMemory();
		}
	}
	threadFlag = false;
	worker.join();
}


static void benchmarkBranchless(benchmark::State& s) {
	int results[5] = { rand(), rand(), rand(), rand(), rand() };
	int a = rand(), b = rand();
	branchlessCacheWarming();
	std::thread worker(runBranchless);
	for (auto _ : s) {
		for (int i = 0; i < A; i++) {
			results[flag] += branch.branch(a,b);
			benchmark::DoNotOptimize(results);
			benchmark::ClobberMemory();	
		}
	}
	threadFlag = false;
	worker.join();
}


//BENCHMARK(benchmarkBranchless);
BENCHMARK(benchmarkBranch);

BENCHMARK_MAIN();