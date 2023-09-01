# Semi-static Conditions

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/maxlucuta/semi-static-conditions/blob/main/LICENSE)
![C++ Version](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![C++ Version](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![CMake](https://img.shields.io/badge/built%20with-CMake-orange.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey.svg)

## Overview

Semi-static conditions emulate traditional conditional statements, but seperate condition evaluation logic and branch taking into two distinct methods. This seperation produces an important decoupling between
relatively cheap and expensive operations,  allowing for more strategic and granular control over conditional branching in low latency settings. Semi-static conditions offers superior branch-taking latencies
and standard deviations to conditional statements when misprediction rates are high. 

```c++
#include <branch.hpp>

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
This project grew out of an MSc research project run by Imperial College London carried out by Max Lucuta and supervised by Dr Paul Bilokon. 
This project is based on an idea by Erez Shermer - Founder, CTO and Managing Member of qSpark, a leading provider of ultra low latency trading platforms for high-frequency algorithmic trading.

## Requirements

The library can only be used with C++17 and onwards due to various template deduction schemes used.

The libarary is available on more recent versions of the following compilers:

* GCC 11.2
* Clang 14.0
* MSVC 16.1

The library is exclusive to x86-64 and ARM-64 architectures only.

## Installation

This describes the installation process using cmake. As pre-requisites, you'll need git and cmake installed.

```bash
# Check out the library.
$ git clone https://github.com/maxlucuta/semi-static-conditions.git
# Go to the library root directory
$ cd semi-static-conditions
# Make a build directory to place the build output.
$ cmake -E make_directory "build"
# Generate build system files with cmake.
$ cmake -E chdir "build" cmake ../
# Build the library.
$ cmake --build "build"
```
This builds the `branch` library. On a unix system, the build directory should now look something like this:

```
/semi-static-conditions
  /build
    /libbranch.a
      ...
  ...
```

Next, you can run the tests to check the build.
```bash
$ cmake -E chdir "build" ctest
```


## Basic Usage

Semi-static conditions behave as an abstraction to conditional statements and switch statements, but remove the runtime cost associated with branch-taking.
To start off using this, define a set of functions that are your 'branches', these must all have identical signatures to for the construct to work.

```c++
void if_branch() { ... }
void else_branch() { ... }

BranchChanger my_branch(if_branch, else_branch);
```

As the programmer, you will have complete control over which branch is executed. For our example, setting the branch direction to `true` will mean that 
when the `branch` method is called, `if_branch` will be executed and vice versa. To execute the branch (i.e, either of the functions passed to the constructor),
simply call the branch method and pass in any arguments, or capture any return values, as if you where using  `if_branch` or  `else_branch` in isolation!

```c++
branch.set_direction(true);
// Executes if_branch
branch.branch();
branch.set_direction(false);
// Executes else_branch
branch.branch();
```
The equivalent code using conditional statements would be:

```c++
if (condition)
  if_branch();
else
  else_branch();
```
This can be extended to any number of branches, providing that the function signatures are all the same. Instead of using booleans, integers can be used to select
the branch direction.

```c++
//                               0        1       2            n-1
BranchChanger switch_statement(func_1, func_2, func_3, ... , func_n);
branch.set_direction(0);
// func_1 executed
branch.branch();
branch.set_direction(2);
// func_3 executed
branch.branch();
```
The former examples work for regular functions or static member functions for classes. For templated functions, programmers are required to generate a pointer
before passing it into `BranchChanger`. However semi-static-conditions also work for non-static member functions!

```c++
class Foo {
  public:
    void func_1() { ... }
    void func_2() { ... }
  ...
};
```
Pass the function pointers to `BranchChanger` as such:

```c++
BranchChanger class_branch(&Foo::func_1, &Foo::func_2);
```
Note that both branches must belong to the same class for this to work. To execute the branches, set the direction as usual and pass an instance of the class before any
arguments. Different instances can be passed through the same `branch` method:

```c++
Foo foo_1("hello");
Foo foo_2("world");
...
class_branch.set_direction(condition);
class_branch.branch(foo_1);
class_branch.branch(foo_2);
```
To use semi-static-conditions, compile and link against the `branch` library (libbranch.a). If you followed the build steps above, this library will 
be under the build directory you created.

```bash
# Example on linux after running the build steps above. Assumes the
# `semi-static-conditions` and `build` directories are under the current directory.
$ g++ mycode.cpp -std=c++17 -isystem semi-static-conditions/include \
      -Lsemi-static-conditions/build -lbranch -o mycode
```

Each time the `BranchChanger` construct is instantiated, permissions on executable pages may be changed to RWX for the duration of the process. If secuirty is of concern, using the `-DSAFE_MODE` flag will ensure page permissions are changed briefly for assembly editing and then reverted back to read-only. This will increase the cost of `set_direction`.

## Acknowledgements

Thank you to Erez Shermer, Founder, CTO \& MM at qSpark for proposing and formulating the project. Also a big thank you to Dr Paul Bilokon, Jonathan Keinan, Lior Keren, Nataly Rasovsky, Nimrod Sapir, Michael Stevenson, and other
colleagues at qSpark for the supervision and constructive suggestions throughout the project.

