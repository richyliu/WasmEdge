except_no_fuzzer: test run preload.so

all: except_no_fuzzer run_no_fuzzer

test: test.cpp
	clang++-12 $< -g -pie -fPIE -lwasmedge_c -I../build/include/api -L../build/lib/api/ -Wl,-rpath="${PWD}/../build/lib/api" -fsanitize=fuzzer,address -o $@

run: run.cpp
	clang++-12 $< -g -pie -fPIE -lwasmedge_c -I../build/include/api -L../build/lib/api/ -Wl,-rpath="${PWD}/../build/lib/api" -fsanitize=fuzzer-no-link,address -o $@

run_no_fuzzer: run.cpp
	clang++-12 $< -g -pie -fPIE -lwasmedge_c -I../build/include/api -L../build/lib/api/ -Wl,-rpath="${PWD}/../build/lib/api" -o $@

preload.so: preload.c
	clang-12 $< -shared -fPIC -o $@

clean:
	rm -f test run run_no_fuzzer preload.so
