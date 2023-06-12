LIB_DIR = ../benchmark
INCLUDE_DIR = $(LIB_DIR)/include
BUILD_DIR = $(LIB_DIR)/build/src

main: bench_hybrid.cpp
	g++ bench_hybrid.cpp -std=c++2a -isystem $(INCLUDE_DIR) -L$(BUILD_DIR) -lbenchmark -lpthread -O3 -o main