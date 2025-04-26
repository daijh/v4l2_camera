# v4l2_player

A C++ command-line application to capture video from a V4L2 device and render it using SDL2.

## Overview

The `v4l2_player` program connects to a specified V4L2 video capture device, configures it for a given format (YUYV in this case), captures frames and displays the video stream in an SDL2 window.

This tool is useful for testing V4L2 device functionality, verifying video pipelines, and demonstrating V4L2 capture and SDL2 rendering integration.

## Usage

```shell
# Help
./v4l2_player -h

Usage:
  ./v4l2_player [OPTION...]

  -h, --help        Print help
  -i, --input arg   Specify capture device (default: /dev/video0)
      --width arg   Specify capture video width (default: 640)
      --height arg  Specify capture video height (default: 360)
      --dmabuf      V4L2 capture device exports DMABUF (default: false)

# Basic usage (uses default /dev/video0, 640x360)
./v4l2_player -i /dev/video0 --width 640 --height 360

# Enable V4L2 capture device DMABUF export
./v4l2_player -i /dev/video0 --width 640 --height 360 --dmabuf
```
