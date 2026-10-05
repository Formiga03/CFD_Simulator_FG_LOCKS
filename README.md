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

## How to Test

- Build with 
 + `cmake -B build -DCMAKE_BUILD_TYPE=Debug`
 + `cmake --build build -j`

- To only Build certain tests:
 + `cmake --build build --target test_name`

- Run test with
 + `ctest --test-dir build --output-on-failure`

- Run Specific Test Tag
 + `./build/tests/test_matrix "[math]"`

- Run Specific Test By Wildcard
 + `./build/tests/test_matrix "Column view*"`

- For Verbose Output
 + `./build/tests/test_matrix -s`