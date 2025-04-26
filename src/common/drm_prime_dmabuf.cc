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
#include <i915_drm.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <xf86drm.h>

#include "check.h"
#include "drm_prime_dmabuf.h"

int DrmPrimeDmabuf::OpenDrm(const char* dev_path) {
  int fd = open(dev_path, O_RDWR);
  if (fd < 0) {
    std::cout << "Unable to open drm device " << dev_path << std::endl;
    CHECK(0);
  }

  drmVersion* version = drmGetVersion(fd);
  if (!version) {
    std::cout << "drmGetVersion failed\n";
    CHECK(0);
  }

  std::cout << dev_path << ": name " << version->name << ", desc "
            << version->desc << std::endl;

  drmFreeVersion(version);

  return fd;
}

DrmPrimeDmabuf::DrmPrimeDmabuf(int drm_fd, int size) {
  int ret;

  CHECK(drm_fd > 0);
  m_drm_fd = drm_fd;

  CHECK(size > 0);
  m_size = size;

  drm_i915_gem_create gem_create = {};
  gem_create.size = m_size;
  ret = drmIoctl(m_drm_fd, DRM_IOCTL_I915_GEM_CREATE, &gem_create);
  if (ret) {
    std::cout << "DRM_IOCTL_I915_GEM_CREATE failed: size " << gem_create.size
              << std::endl;
    CHECK(0);
  }

  drm_i915_gem_set_tiling gem_set_tiling = {};
  gem_set_tiling.handle = gem_create.handle;
  gem_set_tiling.tiling_mode = I915_TILING_NONE;
  ret = drmIoctl(m_drm_fd, DRM_IOCTL_I915_GEM_SET_TILING, &gem_set_tiling);
  if (ret) {
    std::cout << "DRM_IOCTL_I915_GEM_SET_TILING failed\n";
    CHECK(0);
  }

  ret = drmPrimeHandleToFD(m_drm_fd, gem_create.handle, DRM_CLOEXEC | DRM_RDWR,
                           &m_fd);
  if (ret != 0) {
    std::cout << "drmPrimeHandleToFD failed\n";
    CHECK(0);
  }

  return;
}

void* DrmPrimeDmabuf::Map(uint32_t size) {
  CHECK(m_size >= size);

  m_mapped_addr = mmap(0, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);

  CHECK(m_mapped_addr != nullptr);
  CHECK(m_mapped_addr != MAP_FAILED);

  return m_mapped_addr;
}

void DrmPrimeDmabuf::Unmap(void* addr) {
  CHECK(m_mapped_addr == addr);

  munmap(m_mapped_addr, m_size);
  m_mapped_addr = nullptr;
}
