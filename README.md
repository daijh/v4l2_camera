# v4l2_camera

[![V4L2](https://img.shields.io/badge/V4L2-Video4Linux2-orange)](https://linuxtv.org/downloads/v4l-utils/manual/html/v4l2-intro.html)
[![C++](https://img.shields.io/badge/C++-blueviolet?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-brightgreen?logo=cmake&logoColor=white)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://www.linux.org/)

A collection of sample applications and utilities for working with V4L2 (Video4Linux2) devices on Linux.

## Overview

This repository contains several small, focused programs demonstrating different aspects of interacting with V4L2 devices, including device information, video capture, video output, and rendering using SDL2.

* [`v4l2_info`](src/v4l2_info)
* [`v4l2_player`](src/v4l2_player)
* [`v4l2_clone_device`](src/v4l2_clone_device)
* [`sdl2_renderer`](src/sdl2_renderer)

## Getting Started

1. **Prerequisites:** Install necessary development packages.

    ```shell
    # For Debian/Ubuntu
    sudo apt update
    sudo apt install libsdl2-dev libyuv-dev libdrm-dev cmake build-essential
    ```

2. **Clone the Repository:**

    ```shell
    git clone https://github.com/daijh/v4l2_camera.git
    cd v4l2_camera
    ```

3. **Build the Project:**

    ```shell
    cmake -S . -B build
    cmake --build build -j8
    ```

4. **Run:**

    ```shell
    cd build/out
    ./v4l2_player
    ```
