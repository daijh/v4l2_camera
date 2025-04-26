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

#include <vector>

#include <libyuv.h>

#include "check.h"
#include "sdl2_video_renderer.h"

void SDL2HandleEvent() {
  SDL_Event event;
  if (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
          case SDLK_q:
            CHECK(0);
            break;
          default:
            break;
        }
        break;
      case SDL_QUIT:
        CHECK(0);
        break;
      default:
        break;
    }
  }
}

SDL2VideoRenderer::SDL2VideoRenderer(const char* name,
                                     const int window_width,
                                     const int window_height)
    : m_window_width(window_width), m_window_height(window_height) {
  CHECK(name);
  CHECK(window_width);
  CHECK(window_height);
  // SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

  if (!SDL_WasInit(SDL_INIT_VIDEO)) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cout << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
      CHECK(0);
    }
  }

  m_window =
      SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       m_window_width, m_window_height, SDL_WINDOW_SHOWN);
  if (!m_window) {
    std::cout << "Could not create SDL window: " << SDL_GetError() << std::endl;
    CHECK(0);
  }
  std::cout << "Create window " << m_window_width << "x" << m_window_height
            << std::endl;

  m_renderer = SDL_CreateRenderer(m_window, -1, 0);
  if (!m_renderer) {
    std::cout << "Could not create SDL renderer: " << SDL_GetError()
              << std::endl;
    CHECK(0);
  }
}

SDL2VideoRenderer::~SDL2VideoRenderer() {
  if (m_texture) {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
  }

  if (m_renderer) {
    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;
  }

  if (m_window) {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }

  if (SDL_WasInit(SDL_INIT_VIDEO)) {
    SDL_Quit();
  }
}

void SDL2VideoRenderer::RenderFrameI420(uint32_t width,
                                        uint32_t height,
                                        const uint8_t* y_data,
                                        uint32_t y_pitch,
                                        const uint8_t* u_data,
                                        uint32_t u_pitch,
                                        const uint8_t* v_data,
                                        uint32_t v_pitch) {
  int ret;

  if (!m_texture) {
    std::cout << "Create texture " << width << "x" << height << std::endl;

    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_IYUV,
                                  SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!m_texture) {
      std::cout << "Could not create SDL texture: " << SDL_GetError()
                << std::endl;
      CHECK(0);
    }
  }

  ret = SDL_UpdateYUVTexture(m_texture, nullptr, y_data, y_pitch, u_data,
                             u_pitch, v_data, v_pitch);
  if (ret != 0) {
    std::cout << "Could not update SDL texture: " << SDL_GetError()
              << std::endl;
    return;
  }

  SDL_RenderClear(m_renderer);
  SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
  SDL_RenderPresent(m_renderer);

  m_frame_count++;

  SDL2HandleEvent();
}

void SDL2VideoRenderer::RenderFrameYUY2(uint32_t width,
                                        uint32_t height,
                                        const uint8_t* data,
                                        uint32_t stride) {
  const size_t i420_size = width * height * 3 / 2;
  std::vector<uint8_t> i420_frame(i420_size);
  libyuv::YUY2ToI420(data, stride, i420_frame.data(), width,
                     i420_frame.data() + width * height, width / 2,
                     i420_frame.data() + width * height * 5 / 4, width / 2,
                     width, height);

  RenderFrameI420(width, height, i420_frame.data(), width,
                  i420_frame.data() + width * height, width / 2,
                  i420_frame.data() + width * height * 5 / 4, width / 2);
}

void SDL2VideoRenderer::RenderFrameNV12(uint32_t width,
                                        uint32_t height,
                                        const uint8_t* y_data,
                                        int y_stride,
                                        const uint8_t* uv_data,
                                        int uv_stride) {
  const size_t i420_size = width * height * 3 / 2;
  std::vector<uint8_t> i420_frame(i420_size);
  libyuv::NV12ToI420(y_data, y_stride, uv_data, uv_stride, i420_frame.data(),
                     width, i420_frame.data() + width * height, width / 2,
                     i420_frame.data() + width * height * 5 / 4, width / 2,
                     width, height);

  RenderFrameI420(width, height, i420_frame.data(), width,
                  i420_frame.data() + width * height, width / 2,
                  i420_frame.data() + width * height * 5 / 4, width / 2);
}
