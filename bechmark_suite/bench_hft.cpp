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
#include "branch.h"

#define N 100000

int socketFD;
struct sockaddr_in serverAddress;
const char* buy = "BUY";
const char* sell = "SELL";

void sendBuyOrder(int amount)
{
    const char* amountStr = std::to_string(amount).c_str();
    char orderDesc[strlen(amountStr) + strlen(buy) + 1];
    strcpy(orderDesc, amountStr);
    strcat(orderDesc, buy);
    sendto(
        socketFD, 
        (const char *)orderDesc, 
        strlen(orderDesc),
        MSG_CONFIRM, 
        (const struct sockaddr*) &serverAddress, 
        sizeof(serverAddress)
    );
}

void sendSellOrder(int amount)
{
    const char* amountStr = std::to_string(amount).c_str();
    char orderDesc[strlen(amountStr) + strlen(sell) + 1];
    strcpy(orderDesc, amountStr);
    strcat(orderDesc, sell);
    sendto(
        socketFD, 
        (const char *)orderDesc, 
        strlen(orderDesc),
        MSG_CONFIRM, 
        (const struct sockaddr*) &serverAddress, 
        sizeof(serverAddress)
    );
}

static bool run = true;
static bool flag = true;
static BranchChanger branch(
    sendBuyOrder,
    sendSellOrder
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
        branch.set_direction(flag);
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
    } while (run);
}

static void setup(const benchmark::State& s) {
    flag = true;
    run = true;
    srand(time(NULL));
    serverAddress = {0};
    socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
}

static void benchmarkBranch(benchmark::State& s) {
    int sleep = s.range(0);
    std::thread worker(runBranch, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            int orderAmount = rand();
            if (flag) sendBuyOrder(orderAmount);
            else sendSellOrder(orderAmount);
        }
    }
    run = false;
    worker.join();
    close(socketFD);
}

static void benchmarkBranchless(benchmark::State& s) {
    srand(time(NULL));
    int sleep = s.range(0);
    std::thread worker(runBranchless, sleep);
    for (auto _ : s) {
        for (int i = 0; i < N; i++) {
            int orderAmount = rand();
            branch.branch(orderAmount);
        }
    }
    run = false;
    worker.join();
    close(socketFD);
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

