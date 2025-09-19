#include "device_transfers.hpp"
#include "messageStream.H"

ReusableTtBuffer& copy_scalarField_to_device(KernelLauncher& kernelLauncher, const Foam::scalarField& field) {
    auto* device = kernelLauncher.device_;

    size_t bytes = sizeof(float)*field.size();

    auto& buffer = kernelLauncher.allocateBuffer(bytes);

    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        buffer.buffer,
        field.cdata(),
        {0, round_up(bytes, tt_block_size)},
        false
    );

    return buffer;
}

void copy_scalarField_from_device(KernelLauncher& kernelLauncher, ReusableTtBuffer& buffer, Foam::scalarField& field) {
    auto* device = kernelLauncher.device_;

    size_t bytes = sizeof(float)*field.size();
    size_t padded_bytes = round_up(bytes, tt_block_size);

    float* data = nullptr;
    if (bytes == padded_bytes) {
        data = field.data();
    } else {
        data = new float[padded_bytes/sizeof(float)];
    }

    tt::tt_metal::EnqueueReadSubBuffer(
        device->command_queue(0),
        buffer.buffer,
        data,
        {0, padded_bytes},
        true
    );

    if (bytes != padded_bytes) {
        std::memcpy(field.data(), data, bytes);
        delete[] data;
    }
}

/**
 * For each set Interface :
 *  0: interface idx
 *  1: number of coeffs
 *  2:    coeffs
 *        ...
 *  n: interface idx
 * This allows skipping interfaces that are not set
 */
std::pair<ReusableTtBuffer&, int> copy_interfaceCoeffs_to_device(
    KernelLauncher& kernelLauncher,
    const Foam::FieldField<Foam::Field, Foam::scalar>& interfaceCoeffs,
    const Foam::lduInterfaceFieldPtrsList& interfaces
) {

    auto* device = kernelLauncher.device_;

    size_t total_size = 0;
    auto iface_count = interfaces.size();
    auto used_iface_count = 0;

    for (int i = 0; i < iface_count; ++i) {
        if (interfaces.set(i)) {
            total_size += 4 + 4; // size
            auto size = interfaceCoeffs[i].size();
            total_size += size * sizeof(float);
            used_iface_count++;
        }
    }

    auto& buffer = kernelLauncher.allocateBuffer(total_size);

    std::vector<uint32_t> interface_data(total_size / sizeof(uint32_t));
    size_t idx = 0;
    for (int i = 0; i < iface_count; ++i) {
        if (interfaces.set(i)) {
            interface_data[idx++] = i;
            auto num_coeffs = interfaceCoeffs[i].size();
            interface_data[idx++] = num_coeffs;
        
            for (int j = 0; j < interfaceCoeffs[i].size(); ++j) {
                interface_data[idx++] = reinterpret_cast<const uint32_t&>(interfaceCoeffs[i][j]);
            }
        }
    }

    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        buffer.buffer,
        interface_data.data(),
        {0, total_size},
        false
    );

    return {buffer, used_iface_count};


}
