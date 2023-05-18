/* Application Benchmarking for BranchChanger Construct
   
   Maximilian Lucuta
   Imperial College London

   Application: 

   The proposed construct has two key methods: set_direction(bool), which
   can be thought of a single 'expensive' operation, and branch(), which is
   a single, low cost operation with the overhead of a single assembly
   instruction, in comparison to a direct call. Intuitively, the direction
   changing operation cannot lie within the core loop that evaluates the
   runtime condition, since it has a similar cost to an indirection, which
   defeats the whole purpose of its functionality. In an event based system,
   for example, a system that reads in real time data and flips a flag based
   on some event that is reflected in the data, the set_direction method can
   be positioned within this method which will only change the flag once an
   event occurs.

   In this scenario, the cost of the expensive direction changing operation
   can be 'paid for' by the successive cheap operations of the branch()
   method, and hence become comptetitive with repeating if statements. The
   if statements would need to evaluate the condition every single time,
   and when an event occurs, is likely to mispredict the branch several times
   which costs clock cycles. The goal is to find: how often does an event
   need to occur for the overall cost of using BranchChanger to be less than
   using repeated conditional statements.

   Note:

   The suite below is a crude prototype and bot yet sophisticated enough to
   make confident conclusions on the results to an academic standard.
*/

#include <iostream>
#include <time.h>
#include <vector>
#include <chrono>
#include "branch.h"

#define L1_SIZE 98304
#define ITERATIONS 10000000

void buyOrder();
void sellOrder();

enum Action { BUY = 1, SELL = 0 };

/* Dummy array used to read and write to, specifically used
   to try flush the instruction cache between tests to 
   negate its effect for measurements. */
long* clobber = new long[L1_SIZE];

/* Flag for tracking event changes. */
Action currentEvent = BUY;

/* Dummy vectors used in 'buy' and 'sell' methods for
   demonstartion purposes. */
std::vector<int> buyOrders;
std::vector<int> sellOrders;

/* This array will hold a timeline of events that occur over
   a set number of iterations. It will contain a 1 or a 0, which
   denotes a buy or sell order.*/
std::vector<int> events;

BranchChanger branch = BranchChanger(buyOrder, sellOrder);

/* Helper method to flush instruction cache and initialise data
   structures used in measurements. We populate the event array
   as 'an event occurs every x amount of iterations', which can
   be used as a proxy for 'something happens every x amount of time'. */
__attribute__((optimize("O0")))
void flushIntructionCache() {
    srand(time(NULL));
    for (int i = 0; i < L1_SIZE; i++) { clobber[i] = rand(); }
    for (int i = 0; i < ITERATIONS; i++) { 
        if (i % 1000 == 0) { events.push_back(1); }
        else { events.push_back(0); }
    }
    random_shuffle(std::begin(events), std::end(events));
    buyOrders.clear();
    sellOrders.clear();
}

void buyOrder() {
    int newOrderAmount = rand();
    buyOrders.push_back(newOrderAmount);
}

void sellOrder() {
    int newOrderAmount = rand();
    sellOrders.push_back(newOrderAmount);
}

void eventBranch(int i) {
    if (events[i] != currentEvent) {
        currentEvent = Action(events[i]);
    }
}

void eventBranchless(int i) {
    if (events[i] != currentEvent) {
        currentEvent = Action(events[i]);
        branch.set_direction(currentEvent);
    }
}

long long benchmarkBranch() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; i++) {
        eventBranch(i);
        if (currentEvent == BUY) { buyOrder(); }
        else { sellOrder(); }
    }
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
}

long long benchmarkBranchless() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; i++) {
        eventBranchless(i);
        branch.branch();
    }
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
}


int main() {
    flushIntructionCache();
    auto readingBranched = benchmarkBranch();
    flushIntructionCache();
    auto readingBranchless = benchmarkBranchless();
    std::cout << "Branched execution time: " << readingBranched << std::endl;
    std::cout << "Branchless execution time: " << readingBranchless << std::endl;
    delete [] clobber;
    return 0;
}