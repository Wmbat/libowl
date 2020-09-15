# vermillon

![Linux](https://github.com/Wmbat/vermillon/workflows/Linux/badge.svg)

[vermillon](https://github.com/Wmbat/vermillon) is a small game engine project made using
[Vulkan](https://www.khronos.org/vulkan/) and C++20

## Table of Content
* [Requirements](#requirements)
* [Dependencies](#dependencies)
* [Installation](#installation)
* [License](#license)

## Requirements

You need to use clang-10.0 and above.

## Dependencies

* [Vulkan Headers - 1.2.135](https://github.com/KhronosGroup/Vulkan-Headers)
* [GLFW - 3.3.2](https://github.com/glfw/glfw)
* [GLM - 0.9.9.8](https://github.com/g-truc/glm)
* [spdlog - 1.6.1](https://github.com/gabime/spdlog)
* [glslang - 8.13.3743](https://github.com/KhronosGroup/glslang)
* [SPIRV-Cross - 2020-06-29](https://github.com/KhronosGroup/SPIRV-cross)
* [monads - master](https://github.com/Wmbat/monads)
* [patterns - master](https://github.com/mpark/patterns)

## Installation

To install mélodie, you can clone the project locally using 
```sh
git clone https://github.com/Wmbat/vermillon
```

To compile the project, you can run the local `build.py` script. For finding out the acceptable 
parameters of the script, use 
```sh
python build.py --help
```
The `build.py` script will launch cmake and use 
[CPM.cmake](https://github.com/TheLartians/CPM.cmake) to download all the 
[dependencies](#dependencies). Once cmake is done downloading and setting everything up, it will 
launch [Make](https://www.gnu.org/software/make/) or [Ninja](https://ninja-build.org/) and build 
the project.

## License

> You can find the project's license [here](https://github.com/Wmbat/vermillon/blob/master/LICENSE)

This project is licensed under the terms of the **MIT** license.
