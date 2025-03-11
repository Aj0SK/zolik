CPP = g++
CPP_PERF = -O3 -march=native
CPPFLAGS = -std=c++20 -Wall -Wpedantic $(CPP_PERF)

run:
	$(CPP) $(CPPFLAGS) main.cc && ./a.out

run-tests:
	$(CPP) $(CPPFLAGS) test.cc -lgtest -lgtest_main -lpthread && ./a.out
