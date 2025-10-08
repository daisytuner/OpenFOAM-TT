#pragma once

#include <tt-metalium/host_api.hpp>
#include "ReusableTtBuffer.hpp"

namespace tt::daisy {

class BufferPool {
public:
    tt::tt_metal::IDevice* device_;

protected:
    std::vector<ReusableTtBuffer*> buffers_;

public:
    BufferPool(tt_metal::IDevice* device = tt_metal::CreateDevice(0));
    ~BufferPool();

    ReusableTtBuffer& allocateBuffer(size_t size, size_t page_size);
    void freeBuffer(ReusableTtBuffer& buffer);

};

} // namespace tt::daisy