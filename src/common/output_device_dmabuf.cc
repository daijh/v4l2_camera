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

#include <iostream>

#include <cerrno>

#include "check.h"
#include "output_device_dmabuf.h"

OutputDeviceDmabuf::OutputDeviceDmabuf(int fd, int width, int height)
    : m_fd(fd), m_width(width), m_height(height) {
  std::cout << "OutputDeviceDmabuf\n";
}

OutputDeviceDmabuf::~OutputDeviceDmabuf() {}

void OutputDeviceDmabuf::Initialize(int buffer_count) {
  int ret;

  v4l2_requestbuffers reqbuf = {};
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbuf.memory = V4L2_MEMORY_DMABUF;
  reqbuf.count = buffer_count;

  ret = ioctl(m_fd, VIDIOC_REQBUFS, &reqbuf);
  if (ret != 0) {
    std::cout << "ioctl(VIDIOC_REQBUFS) failed\n";
    CHECK(0);
  }

  m_drm_fd = DrmPrimeDmabuf::OpenDrm("/dev/dri/renderD128");
  CHECK(m_drm_fd > 0);

  m_dmabufs.clear();
  m_device_buffers.clear();
  for (uint32_t i = 0; i < reqbuf.count; i++) {
    v4l2_buffer v4l2_buf = {};
    v4l2_buf.index = i;
    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    v4l2_buf.memory = V4L2_MEMORY_DMABUF;
    if (ioctl(m_fd, VIDIOC_QUERYBUF, &v4l2_buf) < 0) {
      std::cout << "ioctl(VIDIOC_QUERYBUF) failed\n";
      CHECK(0);
    }

    auto dmabuf = std::make_shared<DrmPrimeDmabuf>(m_drm_fd, v4l2_buf.length);
    CHECK(dmabuf->m_size >= v4l2_buf.length);

    V4L2DeviceBuffer device_buffer = {};
    device_buffer.index = i;
    device_buffer.len = dmabuf->m_size;
    device_buffer.data = nullptr;
    m_device_buffers.push_back(device_buffer);

    m_dmabufs.push_back(dmabuf);

    std::cout << "dmabuf fd " << dmabuf->m_fd << std::endl;
  }

  std::cout << "Required buffers " << buffer_count << ", created buffers "
            << reqbuf.count << std::endl;
}

void OutputDeviceDmabuf::Start() {
  for (uint32_t i = 0; i < m_device_buffers.size(); i++) {
    Queue(m_device_buffers[i]);
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if (ioctl(m_fd, VIDIOC_STREAMON, &type) < 0) {
    std::cout << "ioctl(VIDIOC_STREAMON) failed\n";
    CHECK(0);
  }

  std::cout << "Started\n";
}

V4L2DeviceBuffer OutputDeviceDmabuf::Dequeue() {
  int ret;

  v4l2_buffer v4l2_buf = {};
  v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  v4l2_buf.memory = V4L2_MEMORY_DMABUF;

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

  std::shared_ptr<DrmPrimeDmabuf> dmabuf = m_dmabufs[v4l2_buf.index];
  m_device_buffers[v4l2_buf.index].data = (uint8_t*)dmabuf->Map(dmabuf->m_size);

  return m_device_buffers[v4l2_buf.index];
}

void OutputDeviceDmabuf::Queue(V4L2DeviceBuffer device_buffer) {
  if (m_device_buffers[device_buffer.index].data) {
    std::shared_ptr<DrmPrimeDmabuf> dmabuf = m_dmabufs[device_buffer.index];
    dmabuf->Unmap(m_device_buffers[device_buffer.index].data);
    m_device_buffers[device_buffer.index].data = nullptr;
  }

  v4l2_buffer v4l2_buf = {};
  v4l2_buf.index = device_buffer.index;
  v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  v4l2_buf.bytesused = device_buffer.len;
  v4l2_buf.memory = V4L2_MEMORY_DMABUF;
  v4l2_buf.m.fd = m_dmabufs[device_buffer.index]->m_fd;

  if (ioctl(m_fd, VIDIOC_QBUF, &v4l2_buf) < 0) {
    std::cout << "ioctl(VIDIOC_QBUF) failed\n";
    CHECK(0);
  }
}
