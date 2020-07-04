# Epona

![C/C++ CI](https://github.com/Wmbat/Epona/workflows/C/C++%20CI/badge.svg)
[![Documentation](https://codedocs.xyz/Wmbat/Epona.svg)](https://codedocs.xyz/Wmbat/Epona/)

[Epona](https://github.com/Wmbat/Epona) is a small game engine project made using
[Vulkan](https://www.khronos.org/vulkan/) and C++20

## Table of Content
* [Requirements](#requirements)
* [Dependencies](#dependencies)
* [Installation](#installation)
* [License](#license)

## Requirements

Due to a lack of support for C++20 features. The only usable compiler is GCC10.

## Dependencies

* [Vulkan Headers - 1.2.135](https://github.com/KhronosGroup/Vulkan-Headers)
* [GLFW - 3.3.2](https://github.com/glfw/glfw)
* [spdlog - 1.6.1](https://github.com/gabime/spdlog)

## Installation

To install Epona, you can clone the project locally using 
```sh
git clone https://github.com/Wmbat/Epona
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

> You can find the project's license [here](https://github.com/Wmbat/Epona/blob/master/LICENSE)

This project is licensed under the terms of the **MIT** license.
