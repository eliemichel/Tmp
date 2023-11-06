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

#include "save_image.h"

#include <webgpu/webgpu.hpp>

#include <filesystem>
#include <string>

bool saveTexture(const std::filesystem::path path, wgpu::Device device, wgpu::Texture texture) {
	using namespace wgpu;
	uint32_t width = texture.getWidth();
	uint32_t height = texture.getHeight();

	static std::unique_ptr<FileRenderer> renderer = nullptr;
	if (!renderer) {
		renderer = std::make_unique<FileRenderer>(device, width, height);
	}

	return renderer->render(path, texture);
}

std::filesystem::path resolvePath(int frame) {
	std::filesystem::path base = "render/frame" + std::to_string(frame) + ".png";
	create_directories(base.parent_path());
	return std::filesystem::absolute(base);
}

FileRenderer::FileRenderer(wgpu::Device device, uint32_t width, uint32_t height)
	: m_device(device)
	, m_width(width)
	, m_height(height)
{
	using namespace wgpu;

	// Create a buffer to get pixels
	BufferDescriptor pixelBufferDesc = Default;
	pixelBufferDesc.mappedAtCreation = false;
	pixelBufferDesc.usage = BufferUsage::MapRead | BufferUsage::CopyDst;
	pixelBufferDesc.size = 4 * width * height;
	Buffer pixelBuffer = device.createBuffer(pixelBufferDesc);

	m_pixelBuffer = pixelBuffer;
	m_pixelBufferDesc = pixelBufferDesc;
}

bool FileRenderer::render(const std::filesystem::path path, wgpu::Texture texture) const {
	using namespace wgpu;
	auto device = m_device;
	auto width = m_width;
	auto height = m_height;
	auto pixelBuffer = m_pixelBuffer;
	auto pixelBufferDesc = m_pixelBufferDesc;

	// Start encoding the commands
	CommandEncoder encoder = device.createCommandEncoder(Default);

	// Get pixels
	ImageCopyTexture source = Default;
	source.texture = texture;
	ImageCopyBuffer destination = Default;
	destination.buffer = pixelBuffer;
	destination.layout.bytesPerRow = 4 * width;
	destination.layout.offset = 0;
	destination.layout.rowsPerImage = height;
	encoder.copyTextureToBuffer(source, destination, { width, height, 1 });

	// Issue commands
	Queue queue = device.getQueue();
	CommandBuffer command = encoder.finish(Default);
	queue.submit(command);

	encoder.release();
	command.release();

	// Map buffer
	std::vector<uint8_t> pixels;
	bool done = false;
	bool failed = false;
	auto callbackHandle = pixelBuffer.mapAsync(MapMode::Read, 0, pixelBufferDesc.size, [&](BufferMapAsyncStatus status) {
		if (status != BufferMapAsyncStatus::Success) {
			failed = true;
			done = true;
			return;
		}
		unsigned char* pixelData = (unsigned char*)pixelBuffer.getConstMappedRange(0, pixelBufferDesc.size);
		int bytesPerRow = 4 * width;
		int success = stbi_write_png(path.string().c_str(), (int)width, (int)height, 4, pixelData, bytesPerRow);

		pixelBuffer.unmap();

		failed = success == 0;
		done = true;
	});

	// Wait for mapping
	while (!done) {
#ifdef WEBGPU_BACKEND_WGPU
		wgpuQueueSubmit(queue, 0, nullptr);
#else
		device.tick();
#endif
	}

	queue.release();

	return !failed;
}
