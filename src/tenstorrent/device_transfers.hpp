#pragma once

#include "ReusableTtBuffer.hpp"
#include "kernel_launcher.hpp"
#include "lduAddressing.H"
#include "tmp.H"
#include "ttLduData.hpp"
#include <Field.H>
#include <scalarField.H>
#include <FieldField.H>
#include <lduInterfaceFieldPtrsList.H>

namespace tt::daisy::foam {

constexpr size_t tt_block_size = 1024;

ReusableTtBuffer& allocate_field_buffer(
    BufferPool& bufferPool,
    uint32_t num_elements
);

ReusableTtBuffer& allocate_field_buffer(
    BufferPool& bufferPool,
    const Foam::scalarField& field
);

ReusableTtBuffer& allocate_field_buffer_bare(
    BufferPool& bufferPool,
    uint32_t num_elements
);

ReusableTtBuffer& allocate_field_buffer_bare(
    BufferPool& bufferPool,
    const Foam::scalarField& field
);

ReusableTtBuffer& allocate_field_buffer_1tile(
    BufferPool& bufferPool,
    uint32_t num_elements
);

ReusableTtBuffer& allocate_field_buffer_1tile(
    BufferPool& bufferPool,
    const Foam::scalarField& field
);

ReusableTtBuffer& copy_scalarField_to_device(
    BufferPool& bufferPool,
    const Foam::scalarField& field
);

ReusableTtBuffer& copy_scalarField_to_device_bare(
    BufferPool& bufferPool,
    const Foam::scalarField& field
);

ReusableTtBuffer& copy_scalarField_to_device_as_dense_mat(
    BufferPool& bufferPool,
    const Foam::scalarField& field
);

void copy_scalarField_from_device(
    BufferPool& bufferPool,
    std::variant<std::reference_wrapper<tt::tt_metal::Buffer>, std::shared_ptr<tt::tt_metal::Buffer>> buffer,
    Foam::scalarField* field,
    uint32_t buf_offset = 0
);

void copy_scalarField_from_device(
    BufferPool& bufferPool,
    ReusableTtBuffer& buffer,
    Foam::scalarField* field,
    uint32_t buf_offset = 0
);

void copy_scalarField_from_device_bare(
    BufferPool& bufferPool,
    std::variant<std::reference_wrapper<tt::tt_metal::Buffer>, std::shared_ptr<tt::tt_metal::Buffer>> buffer,
    Foam::scalarField* field,
    uint32_t buf_offset = 0
);

void copy_scalarField_from_device_bare(
    BufferPool& bufferPool,
    ReusableTtBuffer& buffer,
    Foam::scalarField* field,
    uint32_t buf_offset = 0
);

void copy_scalarField_from_device_dense_mat(
    BufferPool& bufferPool,
    std::variant<std::reference_wrapper<tt::tt_metal::Buffer>, std::shared_ptr<tt::tt_metal::Buffer>> buffer,
    Foam::scalarField* field,
    uint32_t buf_offset = 0
);

void copy_scalarField_from_device_dense_mat(BufferPool& bufferPool, ReusableTtBuffer& buffer, Foam::scalarField* field, uint32_t buf_offset = 0);

std::pair<ReusableTtBuffer&, int> copy_interfaceCoeffs_to_device(
    BufferPool& bufferPool,
    const Foam::FieldField<Foam::Field, Foam::scalar>& interfaceCoeffs,
    const Foam::lduInterfaceFieldPtrsList& interfaces
);

void copy_ldu_addrs_to_device(
    BufferPool& bufferPool,
    tt_ldu_meta& tt_meta,
    const Foam::lduMatrix* lduMat
);

void copy_ldu_contents_to_device(
    BufferPool& bufferPool,
    tt_ldu_meta& tt_meta,
    const Foam::lduMatrix* lduMat,
    bool reserve_all = false
);

std::tuple<tt_ldu_meta&, bool, bool> ensure_lduMat_on_device_as_ldu(
    BufferPool& bufferPool,
    const class Foam::lduMatrix* lduMat,
    bool reserve_all_parts = false
);

void copy_ldu_to_dense(
    tt::tt_metal::IDevice* device,
    tt_ldu_meta& tt_meta,
    const Foam::lduMatrix* lduMat
);

void copy_ldu_to_ellpack(
    BufferPool& bufferPool,
    tt_ldu_meta& tt_meta,
    const Foam::lduMatrix* lduMat
);

std::tuple<tt_ldu_meta&, bool, bool> ensure_lduMat_on_device(
    BufferPool& bufferPool,
    const class Foam::lduMatrix* lduMat,
    bool is_expand = false
);

std::tuple<bool, bool, bool> copy_ldu_from_dense(
    tt::tt_metal::IDevice* device,
    tt_ldu_meta& tt_meta,
    Foam::scalarField* diagField,
    Foam::scalarField* lowerField,
    Foam::scalarField* upperField,
    const Foam::lduAddressing& lduAddressing
);

std::tuple<bool, bool, bool> copy_ldu_from_ellpack(
    tt::tt_metal::IDevice* device,
    tt_ldu_meta& tt_meta,
    Foam::scalarField* diagField,
    Foam::scalarField* lowerField,
    Foam::scalarField* upperField,
    const Foam::lduAddressing& lduAddressing
);

void copy_ldu_from_device(
    BufferPool& bufferPool,
    tt_ldu_meta& tt_meta,
    Foam::scalarField* diagField,
    Foam::scalarField* lowerField,
    Foam::scalarField* upperField,
    const Foam::lduAddressing& lduAddressing
);

std::tuple<tt_ldu_meta&, ReusableTtBuffer&, ReusableTtBuffer&> prepare_Amul_inputs(
    BufferPool& bufferPool,
    const Foam::lduMatrix& lduMat,
    const Foam::scalarField& psi,
    const Foam::scalarField& Apsi
);

void clear_tmp_field(const Foam::tmp<Foam::scalarField>& tfield);

}  // namespace tt::daisy::foam