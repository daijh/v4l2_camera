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

#ifndef __CAPTURE_DEVICE_MMAP_H__
#define __CAPTURE_DEVICE_MMAP_H__

#include <cstdint>

#include <vector>

#include "v4l2_device.h"

class CaptureDeviceMmap : public V4L2Device {
 public:
  CaptureDeviceMmap(int fd, int width, int height, bool use_expbuf = false);
  ~CaptureDeviceMmap();

  void Initialize(int buffer_count) override;
  void Start() override;

  void Queue(V4L2DeviceBuffer device_buffer) override;
  V4L2DeviceBuffer Dequeue() override;

 private:
  void Queue(uint32_t index);
  V4L2DeviceBuffer DequeueMmap();

  int m_fd;
  int m_width;
  int m_height;

  bool m_use_expbuf;

  std::vector<V4L2DeviceBuffer> m_device_buffers;
};
#endif /* __CAPTURE_DEVICE_MMAP_H__ */
