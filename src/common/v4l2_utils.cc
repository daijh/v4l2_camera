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

#include <poll.h>
#include <sys/ioctl.h>

#include "check.h"
#include "v4l2_utils.h"

static std::string fourcc_to_string(int fourcc) {
  char fourcc_chars[4];
  fourcc_chars[0] = fourcc & 0xff;
  fourcc_chars[1] = (fourcc >> 8) & 0xff;
  fourcc_chars[2] = (fourcc >> 16) & 0xff;
  fourcc_chars[3] = (fourcc >> 24) & 0xff;

  return std::string(fourcc_chars, 4);
}

bool v4l2_set_pix_format(int fd,
                         uint32_t v4l2_type,
                         v4l2_pix_format* pix_format) {
  std::cout << v4l2_get_device_name(fd) << std::endl;

  v4l2_format format = {};
  format.type = v4l2_type;
  format.fmt.pix = *pix_format;

  if (ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
    std::cout << "ioctl(VIDIOC_S_FMT) failed\n";
    CHECK(0);
  }

  if (pix_format->pixelformat != format.fmt.pix.pixelformat) {
    std::cout << "pixelformat not supported "
              << fourcc_to_string(pix_format->pixelformat) << ", expected "
              << fourcc_to_string(format.fmt.pix.pixelformat) << std::endl;
    return false;
  }

  if ((pix_format->width != format.fmt.pix.width) ||
      (pix_format->height != format.fmt.pix.height)) {
    std::cout << "Video Size not supported " << pix_format->width << "x"
              << pix_format->height << ", expected " << format.fmt.pix.width
              << "x" << format.fmt.pix.height << std::endl;
    return false;
  }

  if (v4l2_type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
    if (pix_format->bytesperline != format.fmt.pix.bytesperline) {
      std::cout << "bytesperline not supported " << pix_format->bytesperline
                << ", expected " << format.fmt.pix.bytesperline << std::endl;
      return false;
    }

    if (pix_format->sizeimage != format.fmt.pix.sizeimage) {
      std::cout << "sizeimage not supported " << pix_format->sizeimage
                << ", expected " << format.fmt.pix.sizeimage << std::endl;
      return false;
    }
  }

  CHECK(format.fmt.pix.field != V4L2_FIELD_INTERLACED);

  std::cout << "Set device pix format: "
            << fourcc_to_string(format.fmt.pix.pixelformat) << ", "
            << format.fmt.pix.width << "x" << format.fmt.pix.height
            << ", bytesperline " << format.fmt.pix.bytesperline
            << ", sizeimage " << format.fmt.pix.sizeimage << std::endl;

  *pix_format = format.fmt.pix;
  return true;
}

std::string v4l2_get_device_name(int fd) {
  struct v4l2_capability cap;
  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
    std::cout << "ioctl(VIDIOC_QUERYCAP) failed\n";
    return "";
  }

  return std::string(reinterpret_cast<const char*>(cap.card));
}

bool v4l2_poll(int fd, int events) {
  struct pollfd pfds = {0};
  pfds.fd = fd;
  pfds.events = events;

  int ready = poll(&pfds, 1, -1);
  if (ready == -1) {
    std::cout << "poll failed\n";
    return false;
  }

  return true;
}
