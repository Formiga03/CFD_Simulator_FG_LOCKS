# CFD_Simulator_FG_LOCKS

## Directories

+ src: library .cpp file compilation
+ include: library .h files
+ app: aplication
 - cli: command line main.cpp
+ tests: unit testing

## How to Compile

Compiling `CMakeLists.txt` under construction in all directories

## Contributing

- [Coding guidelines (binding)](CODING_GUIDELINES.md)
- [Git & GitHub guide](guidelines/GITHUB_GUIDE.md)

## How to Unit Tests

- Build with 
 + `cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug`
 + `cmake --build build-debug -j`

- To only Build certain tests:
 + `cmake --build build-debug --target test_name`

- Run test with
 + `ctest --test-dir build-debug --output-on-failure`

- Run Specific Test Tag
 + `./build-debug/tests/unit/test_name "[math]"`

- Run Specific Test By Wildcard
 + `./build-debug/tests/unit/test_name "Column view*"`

- For Verbose Output
 + `./build-debug/tests/unit/test_name -s`

## How to Speed/Benchmark Tests

- Build with
 + `cmake -B build-release -DCMAKE_BUILD_TYPE=Release`
 + `cmake --build build-release -j`

- Run benchmark with
 + `./build-release/tests/speed/stest_name`

## Extra Info

Testing uses "catch2" and it will try to compile from github to make compilation faster on Linux machines execute:
 `sudo apt update && sudo apt install catch2` 
and the cmake will link directly to the installed directory with no ompilling need.