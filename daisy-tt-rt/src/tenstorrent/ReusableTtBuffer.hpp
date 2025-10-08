#pragma once

#include <tt-metalium/buffer.hpp>
#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

struct ReusableTtBuffer {
    bool free;
    std::shared_ptr<tt::tt_metal::Buffer> buffer;

    ReusableTtBuffer(size_t size, size_t page_size, tt::tt_metal::IDevice* device): free(false) {
        buffer = tt::tt_metal::CreateBuffer({
            .device = device,
            .size = size,
            .page_size = page_size,
            .buffer_type = tt::tt_metal::BufferType::DRAM
        });
    }

private:
    ReusableTtBuffer(): free(false), buffer(nullptr) {}

public:
    static ReusableTtBuffer& unusedPlaceholder() {
        static ReusableTtBuffer null_buffer;
        return null_buffer;
    }

};

}  // namespace tt::daisy