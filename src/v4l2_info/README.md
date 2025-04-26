# v4l2_info

A lightweight command-line tool for displaying information about V4L2 devices connected to system.

## Overview

`v4l2_info` is a simple utility to query Video4Linux2 (V4L2) devices and present their capabilities, formats, and other relevant information in a human-readable format. This can be useful for debugging camera setups, verifying driver functionality, or simply understanding the capabilities of video capture devices.

## Features

* Lists available V4L2 devices (e.g., /dev/video0, /dev/video1, etc.).
* Displays device capabilities (capture, output, etc.).
* Enumerates supported pixel formats and frame sizes for capture streams.
* Shows supported frame rates for specific formats and sizes.

## Usage

```shell
./v4l2_info
```
