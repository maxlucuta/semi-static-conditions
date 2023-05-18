/* Maximilian Lucuta - Benchmark

   Scenario - Suppose we have a real time stream of orders, either a BUY or SELL, which
   periodically changes based on some real time event. In both cases, a worker thread 
   will run in a continuous loop, which sleeps for some random amount of time, and changing
   the global action (if already a BUY, changes to SELL ect).
*/


#include <chrono>
#include <iostream>
#include <vector>
#include <time.h>
#include <thread>
#include "branch.h"


enum Action { BUY = 1, SELL = 0 };

struct Order {
    unsigned long amount;
    Action action;
};


long long pendingOrders = 0;
bool threadRun = true;
Action currentEvent = BUY;
std::vector<int> eventTimesBranch;
std::vector<int> eventTimesBranchless;


void setUpEvent() {
    srand(time(NULL));
    for (int i = 0; i < 300; i++) {
        int sleepTime = rand() % 10 + 1;
        eventTimesBranch.push_back(sleepTime);
        eventTimesBranchless.push_back(sleepTime);
    }
}

void buyOrder() {
    unsigned long amount = rand() % 10000;
    Order order = { amount, BUY };
    pendingOrders++;
}

void sellOrder() {
    unsigned long amount = rand() % 10000;
    Order order = { amount, SELL };
    pendingOrders++;
}

BranchChanger branch = BranchChanger(buyOrder, sellOrder);

void realTimeOrderStreamBranch() {

    while (threadRun) {
        int sleepTime = eventTimesBranch.back();
        eventTimesBranch.pop_back();
        std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
        if (currentEvent == BUY) { currentEvent = SELL; }
        else { currentEvent = BUY; }
    }
}

void realTimeOrderStreamBranchless() {

    while (threadRun) {
        int sleepTime = eventTimesBranchless.back();
        eventTimesBranchless.pop_back();
        std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
        if (currentEvent == BUY) {
            branch.set_direction(SELL);
            currentEvent = SELL;
        } else {
            branch.set_direction(BUY);
            currentEvent = BUY; 
        }
    }
}


void benchmarkBranch() {

    std::thread worker(realTimeOrderStreamBranch);
    auto finish = std::chrono::system_clock::now() + std::chrono::seconds{300};

    while (std::chrono::system_clock::now() < finish) {
        if (currentEvent == BUY) { buyOrder();
        } else { sellOrder(); }
    }
    threadRun = false;
    std::cout << pendingOrders << std::endl;
    worker.join();
}

void benchmarkBranchless() {

    std::thread worker(realTimeOrderStreamBranchless);
    auto finish = std::chrono::system_clock::now() + std::chrono::seconds{300};
    while (std::chrono::system_clock::now() < finish) { branch.branch(); }
    threadRun = false;
    std::cout << pendingOrders << std::endl;
    worker.join();
}


int main() {
    setUpEvent();
    benchmarkBranch();
    pendingOrders = 0;
    threadRun = true;
    benchmarkBranchless();
    return 0;
}