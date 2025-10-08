#include "buffer_pool.hpp"

namespace tt::daisy {

BufferPool::BufferPool(tt_metal::IDevice* device):
        device_(device)
{
    
}

BufferPool::~BufferPool() {
    for (auto buffer : buffers_) {
        delete buffer;
    }
}

ReusableTtBuffer& BufferPool::allocateBuffer(size_t size, size_t page_size) {
    bool found = false;
    auto it = buffers_.begin();
    ReusableTtBuffer* cur = nullptr;
    while (!found && it != buffers_.end()) {
        cur = *it;
        if (cur->free && cur->buffer->size() >= size && cur->buffer->page_size() == page_size) {
            found = true;
            cur->free = false;
            break;
        }
        ++it;
    }

    if (!found) {
        auto buf = new ReusableTtBuffer(tt::round_up(size, page_size), page_size, device_);
        buffers_.push_back(buf);
        return *buf;
    } else {
        return *cur;
    }
}

void BufferPool::freeBuffer(ReusableTtBuffer& buffer) {
    if (buffer.buffer) {
        buffer.free = true;
    }
}

} // namespace tt::daisy