#include <thread>
#include <bits/stdc++.h>

namespace simple {
    int add(int a, int b) { return a + b; }
    int sub(int a, int b) { return a - b; }
    int mul(int a, int b) { return a * b; }
    int div(int a, int b) { return a / b; }
    int mod(int a, int b) { return a % b; }
};

void eventHappens(std::atomic<bool>& flag) { flag = !flag; }
void eventHappens(bool& flag) { flag = !flag; }

void waitForNextEvent(const int64_t& sleep) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep));
}

bool random(const int64_t& min, const int64_t& max) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(min, max);
  return dis(gen);
}

