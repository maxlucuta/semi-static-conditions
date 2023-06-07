#include <time.h>
#include <thread>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <benchmark/benchmark.h>
#include "dummy_exchange.h"
#include "../../branch.h"

#define N 100000


static Exchange NYSE;
static bool run = true;
static bool flag = true;
static BranchChangerClass branch(
    &Exchange::sendBuyOrder,
    &Exchange::sendSellOrder
);


static void runBranch(int sleep) {
    do {
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (run);
}

static void runBranchless(int sleep) {
    do {
        flag = !flag;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (run);
}

static void setup(const benchmark::State& s) {
    flag = true;
    run = true;
    srand(time(NULL));
}

static void benchmarkBranch(benchmark::State& s) {
    int sleep = s.range(0);
    std::thread worker(runBranch, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            int orderAmount = rand();
            if (flag) NYSE.sendBuyOrder(orderAmount);
            else NYSE.sendSellOrder(orderAmount);
        }
    }
    run = false;
    worker.join();
}


BENCHMARK(benchmarkBranch)
    ->DenseRange(1,10)
    ->DenseRange(20,100,10)
    ->DenseRange(200,1000,100)
    ->DenseRange(2000,10000,1000)
    ->DenseRange(20000,100000,10000)
    ->Setup(setup)
    ->Repetitions(10)
    ->ReportAggregatesOnly(true)
    ->Unit(benchmark::kMillisecond);


static void benchmarkBranchless(benchmark::State& s) {
    srand(time(NULL));
    int sleep = s.range(0);
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            int orderAmount = rand();
            branch.branch(NYSE, orderAmount);
        }
    }
    run = false;
    worker.join();
}


BENCHMARK(benchmarkBranchless)
    ->DenseRange(1,10)
    ->DenseRange(20,100,10)
    ->DenseRange(200,1000,100)
    ->DenseRange(2000,10000,1000)
    ->DenseRange(20000,100000,10000)
    ->Setup(setup)
    ->Repetitions(10)
    ->ReportAggregatesOnly(true)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

