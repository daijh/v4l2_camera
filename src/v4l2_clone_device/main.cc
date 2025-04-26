// BSD 3-Clause License
//
// Copyright (c) 2025, Jianhui Dai
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <iostream>
#include <memory>
#include <vector>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <SDL2/SDL.h>

#include <cxxopts.hpp>

#include "capture_device_mmap.h"
#include "check.h"
#include "output_device_dmabuf.h"
#include "output_device_mmap.h"
#include "sdl2_video_renderer.h"
#include "v4l2_utils.h"

struct Config {
  std::string capture_device;
  uint32_t video_width;
  uint32_t video_height;

  std::string output_device;

  bool dmabuf;

  bool not_show_capture;
};

bool g_quit = false;
void sighandler(int) {
  if (!g_quit) {
    g_quit = true;
    signal(SIGINT, SIG_DFL);
    std::cout << "Quit\n";
  }
}

void ParseCommandLine(int argc, char** argv, Config& config) {
  try {
    std::string program_name = argv[0];
    cxxopts::Options options(program_name, "");

    options.add_option("", {"h, help", "Print help"});

    options.add_option(
        "", {"i, input", "Specify capture device",
             cxxopts::value<std::string>()->default_value("/dev/video0")});
    options.add_option("", {"width", "Specify capture video width",
                            cxxopts::value<uint32_t>()->default_value("640")});
    options.add_option("", {"height", "Specify capture video height",
                            cxxopts::value<uint32_t>()->default_value("360")});
    options.add_option(
        "", {"o, output", "Specify output device",
             cxxopts::value<std::string>()->default_value("/dev/video2")});
    options.add_option(
        "",
        {"dmabuf", "Use DMABUF for output device enqueuing (default: false)",
         cxxopts::value<bool>()->default_value("false")->implicit_value(
             "true")});
    options.add_option(
        "", {"not_show", "Do not show capture stream",
             cxxopts::value<bool>()->default_value("false")->implicit_value(
                 "true")});

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    config.capture_device = result["input"].as<std::string>();
    config.video_width = result["width"].as<uint32_t>();
    config.video_height = result["height"].as<uint32_t>();
    config.output_device = result["output"].as<std::string>();
    config.dmabuf = result["dmabuf"].as<bool>();
    config.not_show_capture = result["not_show"].as<bool>();
  } catch (const cxxopts::exceptions::exception& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(-1);
  }
}

int main(int argc, char* argv[]) {
  constexpr uint32_t kPixFormat = V4L2_PIX_FMT_YUYV;
  constexpr uint32_t kBufferCount = 10;

  Config config;
  ParseCommandLine(argc, argv, config);

  std::cout << "======" << std::endl;
  std::cout << "capture_device: " << config.capture_device << std::endl;
  std::cout << "not_show_capture: " << config.not_show_capture << std::endl;
  std::cout << "video_width: " << config.video_width << std::endl;
  std::cout << "video_height: " << config.video_height << std::endl;
  std::cout << "output_device: " << config.output_device << std::endl;
  std::cout << "dmabuf: " << config.dmabuf << std::endl;

  // Open and initialize capture device
  std::cout << "======" << std::endl;
  int capture_fd = open(config.capture_device.c_str(), O_RDWR);
  if (capture_fd < 0) {
    std::cout << "Invalid device: " << config.capture_device << std::endl;
    return -1;
  }

  v4l2_pix_format capture_pix_format = {};
  capture_pix_format.pixelformat = kPixFormat;
  capture_pix_format.width = config.video_width;
  capture_pix_format.height = config.video_height;

  if (!v4l2_set_pix_format(capture_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE,
                           &capture_pix_format)) {
    return -1;
  }

  // Create capture v4l2 device
  std::unique_ptr<V4L2Device> capture = std::make_unique<CaptureDeviceMmap>(
      capture_fd, config.video_width, config.video_height, false);
  capture->Initialize(kBufferCount);
  capture->Start();

  // Open and initialize output device
  std::cout << "======" << std::endl;
  int output_fd = open(config.output_device.c_str(), O_RDWR);
  if (output_fd < 0) {
    std::cout << "Invalid device: " << config.output_device << std::endl;
    return -1;
  }
  // Set capture pix format to output device
  [[maybe_unused]] v4l2_pix_format output_pix_format = capture_pix_format;
  if (!v4l2_set_pix_format(output_fd, V4L2_BUF_TYPE_VIDEO_OUTPUT,
                           &capture_pix_format)) {
    return -1;
  }

  // Create output v4l2 device
  std::unique_ptr<V4L2Device> output;
  if (config.dmabuf) {
    output = std::make_unique<OutputDeviceDmabuf>(output_fd, config.video_width,
                                                  config.video_height);
  } else {
    output = std::make_unique<OutputDeviceMmap>(output_fd, config.video_width,
                                                config.video_height);
  }
  output->Initialize(kBufferCount);
  output->Start();

  // Create renderer with default windows 640x360
  std::unique_ptr<SDL2VideoRenderer> renderer;
  if (!config.not_show_capture) {
    renderer = std::make_unique<SDL2VideoRenderer>(
        v4l2_get_device_name(capture_fd).c_str());
  }

  uint32_t frames = 0;
  signal(SIGINT, sighandler);
  while (!g_quit) {
    // Acquire capture buffer
    if (!v4l2_poll(capture_fd, POLLIN)) {
      std::cout << "Capture device stopped!\n";
      break;
    }
    V4L2DeviceBuffer capture_buffer = capture->Dequeue();

    // Acquire output buffer
    if (!v4l2_poll(output_fd, POLLOUT)) {
      std::cout << "Output device stopped!\n";
      break;
    }
    V4L2DeviceBuffer output_buffer = output->Dequeue();

    // Copy video frame
    CHECK(output_buffer.len >= capture_buffer.len);
    memcpy(output_buffer.data, capture_buffer.data, capture_buffer.len);

    // Return output buffer
    output->Queue(output_buffer);

    // Render
    if (renderer) {
      renderer->RenderFrameYUY2(config.video_width, config.video_height,
                                (uint8_t*)capture_buffer.data,
                                capture_pix_format.bytesperline);
    }

    // Return capture buffer
    capture->Queue(capture_buffer);

    ++frames;
    if (frames % 100 == 0) {
      std::cout << "Frames " << frames << std::endl;
    }
  }

  // Clean up
  capture.reset();
  output.reset();
  renderer.reset();
  return 0;
}
