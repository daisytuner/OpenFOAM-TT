#pragma once

#include <tt-metalium/buffer.hpp>
#include <tt-metalium/host_api.hpp>

constexpr size_t tt_block_size = 1024;

struct ReusableTtBuffer {
    bool free;
    std::shared_ptr<tt::tt_metal::Buffer> buffer;

    ReusableTtBuffer(size_t size, tt::tt_metal::IDevice* device): free(false) {
        buffer = tt::tt_metal::CreateBuffer({
            .device = device,
            .size = size,
            .page_size = tt_block_size,
            .buffer_type = tt::tt_metal::BufferType::DRAM
        });
    }
};