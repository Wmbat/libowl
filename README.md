# Epona

[![Documentation](https://codedocs.xyz/Wmbat/Epona.svg)](https://codedocs.xyz/Wmbat/Epona/)

[Epona](https://github.com/Wmbat/Epona) is a small game engine project made using
[Vulkan](https://www.khronos.org/vulkan/) and C++20

## Table of Content
* [Installation](#installation)
* [License](#license)

## Installation

To install Epona, you can clone the project locally using 
```sh
git clone https://github.com/Wmbat/Epona
```

To compile the project, you can run the local `build.py` script. For finding out the acceptable parameters of the
script, use 
```sh
python build.py --help
```
The `build.py` script will launch cmake and use [CPM.cmake](https://github.com/TheLartians/CPM.cmake) to download all
the [dependencies](#dependencies). Once cmake is done downloading and setting everything up, it will launch
[Make](https://www.gnu.org/software/make/) or [Ninja](https://ninja-build.org/) and build the project.

## License

> You can find the project's license [here](https://github.com/Wmbat/Epona/blob/master/LICENSE)

This project is licensed under the terms of the **MIT** license.
