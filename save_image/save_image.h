/**
 * This file is part of the "Learn WebGPU for C++" book.
 *   https://eliemichel.github.io/LearnWebGPU
 * 
 * MIT License
 * Copyright (c) 2022-2023 Elie Michel
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*

Usage: In the main loop:

{ // export video
	static int frame = 0;
	saveImage(resolvePath(frame), nextTexture);
	++frame;
	if (frame > 200) {
		break;
	}
}

*/

#include <stb_image_write.h>

#include <webgpu/webgpu.hpp>

#include <filesystem>
#include <string>

/**
 * Save a texture to disk
 * NB: Prefer using FileRenderer object to reuse the same buffer
 */
bool saveTexture(const std::filesystem::path path, wgpu::Device device, wgpu::Texture texture);

/**
 * Basic utility to transform frame number into file path (maybe to be removed)
 */
std::filesystem::path resolvePath(int frame);

class FileRenderer {
public:
	FileRenderer(wgpu::Device device, uint32_t width, uint32_t height);

	/**
	 * Texture must have the same resolution that was used to construct the FileRenderer
	 */
	bool render(const std::filesystem::path path, wgpu::Texture texture) const;

private:
	wgpu::Device m_device;
	uint32_t m_width;
	uint32_t m_height;
	wgpu::Buffer m_pixelBuffer = nullptr;
	wgpu::BufferDescriptor m_pixelBufferDesc;
};
