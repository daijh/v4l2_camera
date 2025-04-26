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

#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <signal.h>

#include <SDL2/SDL.h>

#include "sdl2_video_renderer.h"

bool g_quit = false;
void sighandler(int) {
  if (!g_quit) {
    g_quit = true;
    signal(SIGINT, SIG_DFL);
    std::cout << "Quit\n";
  }
}

// Function to fill an I420 buffer with an animated color pattern based on a
// frame number
void FillI420BufferWithAnimatedPattern(
    uint32_t width,
    uint32_t height,
    uint8_t* y_data,
    uint32_t y_pitch,
    uint8_t* u_data,
    uint32_t u_pitch,
    uint8_t* v_data,
    uint32_t v_pitch,
    uint64_t frame_num) {  // Use uint64_t for potentially unlimited numbers
  const uint32_t uv_plane_height = height / 2;  // U and V are half height
  const uint32_t uv_plane_width = width / 2;    // U and V are half width

  // Simple animation based on frame_num
  double time_factor = frame_num * 0.6;  // Adjust speed of animation

  // Fill Y plane (Luminance)
  for (uint32_t i = 0; i < height; ++i) {   // Rows
    for (uint32_t j = 0; j < width; ++j) {  // Columns
      double y_grad = (double)j / width;    // Horizontal gradient
      double anim_effect =
          std::sin(y_grad * M_PI * 4.0 + time_factor);  // Wave effect
      uint8_t y_value = static_cast<uint8_t>(
          std::clamp((y_grad + anim_effect * 0.2) * 255.0, 0.0, 255.0));
      y_data[i * y_pitch + j] = y_value;
    }
  }

  // Fill U and V planes (Chrominance)
  for (uint32_t i = 0; i < uv_plane_height; ++i) {   // Rows (subsampled)
    for (uint32_t j = 0; j < uv_plane_width; ++j) {  // Columns (subsampled)
      double u_grad = (double)i / uv_plane_height;   // Vertical gradient
      double v_grad = (double)j / uv_plane_width;    // Horizontal gradient

      // Combine gradients and time for animated color
      double u_effect = std::cos(u_grad * M_PI * 2.0 + time_factor);
      double v_effect = std::sin(v_grad * M_PI * 2.0 - time_factor);

      uint8_t u_value = static_cast<uint8_t>(
          std::clamp((u_grad + u_effect * 0.3) * 255.0, 0.0, 255.0));
      uint8_t v_value = static_cast<uint8_t>(
          std::clamp((v_grad + v_effect * 0.3) * 255.0, 0.0, 255.0));

      // Center chroma values around 128 for neutral color
      u_data[i * u_pitch + j] =
          static_cast<uint8_t>(std::clamp(u_value, (uint8_t)0, (uint8_t)255));
      v_data[i * v_pitch + j] =
          static_cast<uint8_t>(std::clamp(v_value, (uint8_t)0, (uint8_t)255));
    }
  }
}

int main(int argc, char* argv[]) {
  constexpr uint32_t kVideoWidth = 1280;
  constexpr uint32_t kVideoHeight = 720;

  // Create renderer with default windows 640x360
  std::unique_ptr<SDL2VideoRenderer> renderer =
      std::make_unique<SDL2VideoRenderer>();

  // Allocate buffer for dummy I420 data
  const uint32_t y_plane_size = kVideoWidth * kVideoHeight;
  const uint32_t uv_plane_size = (kVideoWidth / 2) * (kVideoHeight / 2);
  const uint32_t kI420BufferSize = y_plane_size + uv_plane_size + uv_plane_size;
  std::vector<uint8_t> i420_buffer(kI420BufferSize);

  // Calculate pitches (bytes per row)
  const uint32_t y_pitch = kVideoWidth;
  const uint32_t u_pitch = kVideoWidth / 2;
  const uint32_t v_pitch = kVideoWidth / 2;

  // Pointers to the start of each plane within the buffer
  uint8_t* y_data = i420_buffer.data();
  uint8_t* u_data = y_data + y_plane_size;
  uint8_t* v_data = u_data + uv_plane_size;

  // Main loop
  uint64_t frames = 0;
  signal(SIGINT, sighandler);
  while (!g_quit) {
    // Fill the I420 buffer with an animated pattern
    FillI420BufferWithAnimatedPattern(kVideoWidth, kVideoHeight, y_data,
                                      y_pitch, u_data, u_pitch, v_data, v_pitch,
                                      frames++);

    // Render the generated I420 video frame
    renderer->RenderFrameI420(kVideoWidth, kVideoHeight, y_data, y_pitch,
                              u_data, u_pitch, v_data, v_pitch);

    // Sleep 1 second
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Clean up
  renderer.reset();
  return 0;
}
