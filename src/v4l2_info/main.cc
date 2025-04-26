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

#include <cstdint>
#include <cstring>

#include <iostream>
#include <vector>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "check.h"

void ListMemoryTypes(int fd) {
  std::vector<int> memory_types = {V4L2_MEMORY_DMABUF, V4L2_MEMORY_MMAP,
                                   V4L2_MEMORY_USERPTR};
  std::vector<const char*> memory_types_str = {
      "V4L2_MEMORY_DMABUF", "V4L2_MEMORY_MMAP", "V4L2_MEMORY_USERPTR"};

  for (size_t i = 0; i < memory_types.size(); i++) {
    v4l2_requestbuffers reqbuf = {};
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = memory_types[i];
    reqbuf.count = 1;

    int ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
    std::cout << "  " << memory_types_str[i] << ": " << (ret == 0) << std::endl;
  }
}

void ListFrameRate(int fd,
                   uint32_t pixelformat,
                   uint32_t width,
                   uint32_t height) {
  v4l2_frmivalenum vfie = {};
  vfie.pixel_format = pixelformat;
  vfie.width = width;
  vfie.height = height;

  while (!ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &vfie)) {
    CHECK(vfie.type == V4L2_FRMIVAL_TYPE_DISCRETE);
    std::cout << "         frame rate: "
              << float(vfie.discrete.denominator) / vfie.discrete.numerator
              << std::endl;
    vfie.index += 1;
  }
}

void ListFrameSize(int fd, uint32_t pixelformat) {
  v4l2_frmsizeenum vfse = {};
  vfse.pixel_format = pixelformat;

  while (!ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &vfse)) {
    switch (vfse.type) {
      case V4L2_FRMSIZE_TYPE_DISCRETE:
        std::cout << "      " << vfse.discrete.width << "x"
                  << vfse.discrete.height << std::endl;

        ListFrameRate(fd, pixelformat, vfse.discrete.width,
                      vfse.discrete.height);
        break;
      case V4L2_FRMSIZE_TYPE_CONTINUOUS:
      case V4L2_FRMSIZE_TYPE_STEPWISE:
        std::cout << "      " << "Width Range " << vfse.stepwise.min_width
                  << " -> " << vfse.stepwise.max_width << " step "
                  << vfse.stepwise.step_width << ", Height Range "
                  << vfse.stepwise.min_height << " -> "
                  << vfse.stepwise.max_height << " step "
                  << vfse.stepwise.step_height << std::endl;
    }
    vfse.index++;
  }
}

void ListV4L2Device(int fd) {
  v4l2_capability cap = {};
  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
    std::cout << "ioctl(VIDIOC_QUERYCAP): fail\n";
    return;
  }
  std::cout << "  " << "Description: " << cap.card << "," << cap.driver << ","
            << cap.bus_info << std::endl;

  const bool kVideoCapture = (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE);
  const bool kVideoOutput = (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT);
  const bool kReadWrite = (cap.capabilities & V4L2_CAP_READWRITE);
  const bool kStreaming = (cap.capabilities & V4L2_CAP_STREAMING);
  std::cout << "  " << "VIDEO_CAPTURE: " << kVideoCapture << std::endl;
  std::cout << "  " << "VIDEO_OUTPUT: " << kVideoOutput << std::endl;
  std::cout << "  " << "READWRITE: " << kReadWrite << std::endl;
  std::cout << "  " << "STREAMING: " << kStreaming << std::endl;

  if (kVideoCapture) {
    v4l2_fmtdesc vfd = {};
    vfd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    std::cout << "  Supported Formats:\n";
    while (!ioctl(fd, VIDIOC_ENUM_FMT, &vfd)) {
      const bool kCompressed = (vfd.flags & V4L2_FMT_FLAG_COMPRESSED);
      std::cout << "    " << (kCompressed ? "Compressed" : "Raw") << ", "
                << vfd.description << std::endl;

      ListFrameSize(fd, vfd.pixelformat);

      vfd.index++;
    }
  }
}

void ListAll() {
  std::string basename = "/dev/video";

  for (int i = 0; i < 10; i++) {
    std::string dev_name = basename + std::to_string(i);
    int fd = open(dev_name.c_str(), O_RDWR);
    if (fd < 0) {
      continue;
    }

    std::cout << "======\n";
    std::cout << "Open device: " << dev_name.c_str() << std::endl;
    ListV4L2Device(fd);
    ListMemoryTypes(fd);
    close(fd);
  }
}

int main(int argc, char* argv[]) {
  ListAll();
  return 0;
}
