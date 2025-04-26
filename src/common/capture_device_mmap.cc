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

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <cerrno>

#include "capture_device_mmap.h"
#include "check.h"

CaptureDeviceMmap::CaptureDeviceMmap(int fd,
                                     int width,
                                     int height,
                                     bool use_expbuf)
    : m_fd(fd), m_width(width), m_height(height), m_use_expbuf(use_expbuf) {
  std::cout << "CaptureDeviceMmap expbuf " << m_use_expbuf << std::endl;
}

CaptureDeviceMmap::~CaptureDeviceMmap() {}

void CaptureDeviceMmap::Initialize(int buffer_count) {
  int ret;

  v4l2_requestbuffers reqbuf = {};
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = buffer_count;

  ret = ioctl(m_fd, VIDIOC_REQBUFS, &reqbuf);
  if (ret != 0) {
    std::cout << "ioctl(VIDIOC_REQBUFS) failed\n";
    CHECK(0);
  }

  m_device_buffers.clear();
  for (uint32_t i = 0; i < reqbuf.count; i++) {
    v4l2_buffer v4l2_buf = {};
    v4l2_buf.index = i;
    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(m_fd, VIDIOC_QUERYBUF, &v4l2_buf) < 0) {
      std::cout << "ioctl(VIDIOC_QUERYBUF) failed\n";
      CHECK(0);
    }

    V4L2DeviceBuffer device_buffer = {};
    device_buffer.index = i;
    device_buffer.data = nullptr;
    device_buffer.len = v4l2_buf.length;

    if (!m_use_expbuf) {
      device_buffer.data =
          mmap(nullptr, v4l2_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
               m_fd, v4l2_buf.m.offset);
      CHECK(device_buffer.data != MAP_FAILED);
    }
    m_device_buffers.push_back(device_buffer);
  }

  std::cout << "Required buffers " << buffer_count << ", created buffers "
            << reqbuf.count << std::endl;
}

void CaptureDeviceMmap::Start() {
  for (uint32_t i = 0; i < m_device_buffers.size(); i++) {
    Queue(i);
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(m_fd, VIDIOC_STREAMON, &type) < 0) {
    std::cout << "ioctl(VIDIOC_STREAMON) failed\n";
    CHECK(0);
  }

  std::cout << "Started\n";
}

V4L2DeviceBuffer CaptureDeviceMmap::Dequeue() {
  V4L2DeviceBuffer device_buffer = DequeueMmap();

  if (m_use_expbuf) {
    v4l2_exportbuffer expbuf = {};
    expbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    expbuf.index = device_buffer.index;
    if (ioctl(m_fd, VIDIOC_EXPBUF, &expbuf) == -1) {
      std::cout << "ioctl(VIDIOC_EXPBUF) failed\n";
      CHECK(0);
    }

    CHECK(device_buffer.data == nullptr);
    device_buffer.data =
        mmap(nullptr, device_buffer.len, PROT_READ, MAP_SHARED, expbuf.fd, 0);
    CHECK(device_buffer.data != MAP_FAILED);
  }

  return device_buffer;
}

V4L2DeviceBuffer CaptureDeviceMmap::DequeueMmap() {
  int ret;

  v4l2_buffer v4l2_buf = {};
  v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  v4l2_buf.memory = V4L2_MEMORY_MMAP;

  // Attempt to dequeue a buffer, retrying if interrupted by a signal or if
  // temporarily no buffer is available (in some configurations).
  while ((ret = ioctl(m_fd, VIDIOC_DQBUF, &v4l2_buf)) < 0 &&
         ((errno == EINTR) || (errno == EAGAIN))) {
    // retry
  }
  if (ret < 0) {
    std::cout << "ioctl(VIDIOC_DQBUF) failed\n";
    CHECK(0);
  }

  // FIXME: The v4l2loopback driver return zero buffer length
  // m_device_buffers[v4l2_buf.index].len = v4l2_buf.length;
  return m_device_buffers[v4l2_buf.index];
}

void CaptureDeviceMmap::Queue(uint32_t index) {
  v4l2_buffer v4l2_buf = {};
  v4l2_buf.index = index;
  v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  v4l2_buf.memory = V4L2_MEMORY_MMAP;

  if (ioctl(m_fd, VIDIOC_QBUF, &v4l2_buf) < 0) {
    std::cout << "ioctl(VIDIOC_QBUF) failed\n";
    CHECK(0);
  }
}

void CaptureDeviceMmap::Queue(V4L2DeviceBuffer device_buffer) {
  if (m_use_expbuf && device_buffer.data) {
    munmap(device_buffer.data, device_buffer.len);
    device_buffer.data = nullptr;
  }

  Queue(device_buffer.index);
}
