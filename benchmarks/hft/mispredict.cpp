#include <benchmark/benchmark.h>
#include <bits/stdc++.h>
#include "../../src/switch.hpp"
#include "../utilities/utilities.hpp"
#include "../utilities/option_pricing.hpp"

#define A 100000000
#define FREQ 1

static int count = 0;
static std::atomic<int> flag;
static std::atomic<bool> thread;
static BranchChangerSwitch<double, OptionPricing, int, int&> branch(
  blackScholesEuropean,
  binomialEuropean,
  batesCallOptionPrice
);

static void runBranchless() {
  std::srand(std::time(nullptr));
  while (thread) {
    flag = std::rand() % 3;
    branch.setDirection(flag);
    waitForNextEvent(FREQ);
  }
}

static OptionPricing getMarketData() {
  OptionPricing data = {0};
  generateRandomOptionInputs(data);
  return data;
}

static void setup(const benchmark::State& s) {
  flag = std::rand() % 3;
  thread = true;
}

static void benchmarkBranchless(benchmark::State& s) {
  double* results = new double[3];
  std::thread worker(runBranchless);
  for (auto _ : s) {
    for (int i = 0; i < A; i++) {
      s.PauseTiming();
      auto data = getMarketData();
      s.ResumeTiming();
      results[flag] += branch.branch(data, flag, count);
      benchmark::DoNotOptimize(results);
    }
    benchmark::ClobberMemory();
  }
  thread = false;
  worker.join();
  std::cout << "Mispredictions: " << count << std::endl;
}

//BENCHMARK(benchmarkBranch)->Setup(setup);
BENCHMARK(benchmarkBranchless)->Setup(setup);
BENCHMARK_MAIN();
  