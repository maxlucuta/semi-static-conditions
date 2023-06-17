#include <benchmark/benchmark.h>
#include <bits/stdc++.h>
#include "../../src/switch.hpp"
#include "../utilities/utilities.hpp"
#include "../utilities/option_pricing.hpp"

#define A 10000000
#define FREQ 0.9

static std::atomic<int> flag;
static std::atomic<bool> thread;
static BranchChangerSwitch<double, OptionPricing> branch(
  blackScholesEuropean,
  binomialEuropean,
  batesCallOptionPrice
);


static void runBranch() {
  std::srand(std::time(nullptr));
  while (thread) {
    flag = std::rand() % 3;
    waitForNextEvent(FREQ);
  }
}

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
  std::srand(std::time(nullptr));
  flag = std::rand() % 3;
  thread = true;
}

static void benchmarkBranch(benchmark::State& s) {
  double* results = new double[3];
  std::thread worker(runBranch);
  for (auto _ : s) {
    for (int i = 0; i < A; i++) {
      s.PauseTiming();
      auto data = getMarketData();
      s.ResumeTiming();
      if (flag == 0) {
        results[flag] += blackScholesEuropean(data);
      } else if (flag == 1) {
        results[flag] += binomialEuropean(data);
      } else {
        results[flag] += batesCallOptionPrice(data);
      }
      benchmark::DoNotOptimize(results);
    }
    benchmark::ClobberMemory();
  }
  thread = false;
  worker.join();
}

static void benchmarkBranchless(benchmark::State& s) {
  double* results = new double[3];
  std::thread worker(runBranchless);
  for (auto _ : s) {
    for (int i = 0; i < A; i++) {
      s.PauseTiming();
      auto data = getMarketData();
      s.ResumeTiming();
      results[flag] += branch.branch(data);
      benchmark::DoNotOptimize(results);
    }
    benchmark::ClobberMemory();
  }
  thread = false;
  worker.join();
}

BENCHMARK(benchmarkBranch)->Setup(setup);
BENCHMARK(benchmarkBranchless)->Setup(setup);
BENCHMARK_MAIN();
  