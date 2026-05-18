# Graph project

## Building

### Building for Visual Studio (Windows)

This project was developed for Windows under Visual Studio.
To compile this project, make sure you have correctly installed the following tools:
- [Visual Studio](https://visualstudio.microsoft.com/) with "Desktop Development in C++" option,
- [CMake](https://cmake.org/download/).

Open the root folder in Visual Studio to: go coding!

If you wish to compile the project manually, you can run the following commands:
```
mkdir _build_vs
cd _build_vs
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Building for linux

Use the following commands to install project dependencies:
```
sudo apt install cmake
```
To create the project with CMake, run the following commands:
```
mkdir _build_linux _build_linux/debug
cd _build_linux/debug
cmake ../.. -DCMAKE_BUILD_TYPE=Debug
```
You can then compile the project from the `_build_linux/debug` folder with either of the following two commands:
```
cmake --build .
cmake --build . --clean-first
```
Again from the `_build_linux/debug` folder, you can run the binary with the following line
```
../../_bin/Debug/application
```
To run valgrind, use
```
valgrind --leak-check=full --show-leak-kinds=all ../../_bin/Debug/application
```

## License

All code produced for this project is licensed under the [MIT license](/LICENSE.md).
