# Semi-static Conditions

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/your_username/your_library_name/blob/main/LICENSE)
![C++ Version](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![C++ Version](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![CMake](https://img.shields.io/badge/built%20with-CMake-orange.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)
[![Build Status](https://travis-ci.com/your_username/your_library_name.svg?branch=main)](https://travis-ci.com/your_username/your_library_name)
[![Coverage Status](https://coveralls.io/repos/github/your_username/your_library_name/badge.svg?branch=main)](https://coveralls.io/github/your_username/your_library_name?branch=main)

## Overview

Semi-static conditions emulate traditional conditional statements, but seperate condition evaluation logic and branch taking into two distinct methods. This seperation produces an important decoupling between
relatively cheap and expensive operations,  allowing for more strategic and granular control over conditional branching in low latency settings. Semi-static conditions offers superior branch-taking latencies
and standard deviations to conditional statements when misprediction rates are high.

```c++
#include <branch.h>

int add(int a, int b) {
  return a + b;
}

int sub(int a, int b) {
  return a - b;
}

int main() {
  BranchChanger branch(add, sub);
  // Choose branch direction (if or else!)
  branch.set_direction(true);
  // Executes add(1,2) 
  branch.branch(1,2)
  branch.set_direction(false);
  // Executes sub(1,2)
  branch.branch(1,2);
}
```

## Requirements

The library can only be used with C++17 and onwards due to various template deduction schemes used.

The libarary is available on more recent versions of the following compilers:



## Installation

TODO...

```bash
# TODO...
