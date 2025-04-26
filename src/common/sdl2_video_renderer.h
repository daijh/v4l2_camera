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

#ifndef __SDL2_VIDEO_RENDERER_H__
#define __SDL2_VIDEO_RENDERER_H__

#include <cstdio>

#include <string>

#include <SDL2/SDL.h>

class SDL2VideoRenderer {
 public:
  SDL2VideoRenderer(const char* name = "SDL2VideoRenderer",
                    const int window_width = 640,
                    const int window_height = 360);
  ~SDL2VideoRenderer();

  void RenderFrameI420(uint32_t width,
                       uint32_t height,
                       const uint8_t* y_data,
                       uint32_t y_pitch,
                       const uint8_t* u_data,
                       uint32_t u_pitch,
                       const uint8_t* v_data,
                       uint32_t v_pitch);

  void RenderFrameNV12(uint32_t width,
                       uint32_t height,
                       const uint8_t* y_data,
                       int y_stride,
                       const uint8_t* uv_data,
                       int uv_stride);

  void RenderFrameYUY2(uint32_t width,
                       uint32_t height,
                       const uint8_t* data,
                       uint32_t stride);

 private:
  int32_t m_window_width;
  int32_t m_window_height;

  int32_t m_frame_count = 0;

  SDL_Window* m_window = nullptr;
  SDL_Renderer* m_renderer = nullptr;
  SDL_Texture* m_texture = nullptr;
};
#endif /* __SDL2_VIDEO_RENDERER_H__ */
