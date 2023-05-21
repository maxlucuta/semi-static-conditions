main: benchmarkBranch.cpp
	g++ benchmarkBranch.cpp -std=c++2a -isystem benchmark/include \
  -Lbenchmark/build/src -lbenchmark -lpthread -O3 -o main