#include <benchmark/benchmark.h>
#include "../../branch.h"

#define N 10000000


static bool run;
static bool flag;

static void runBranch(int sleep) {
    do {
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(sleep)
        );
        flag = !flag;
        std::this_thread::sleep_for(
            std::chrono::microseconds(1000)
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
        flag = !flag;
        branch.setDirection(flag);
        std::this_thread::sleep_for(
            std::chrono::microseconds(1000)
        );
    } while (run);
}

static void benchmarkBranch(benchmark::State& s)
{

}
