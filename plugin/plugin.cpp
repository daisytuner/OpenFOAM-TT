#include <list>
#include <memory>
#include <sdfg/builder/structured_sdfg_builder.h>
#include <sdfg/codegen/dispatchers/node_dispatcher_registry.h>
#include <sdfg/data_flow/tasklet.h>
#include <sdfg/element.h>
#include <sdfg/function.h>
#include <sdfg/plugins/plugins.h>
#include <sdfg/serializer/json_serializer.h>
#include <sdfg/symbolic/symbolic.h>
#include <sdfg/types/function.h>
#include <sdfg/types/pointer.h>
#include <sdfg/types/scalar.h>
#include <sdfg/types/type.h>
#include <sdfg/data_flow/library_nodes/call_node.h>

#include <string>
#include <vector>

#include "sdfg/structured_sdfg.h"

using namespace sdfg;


std::unique_ptr<StructuredSDFG> create_Amul_in_0() {
    builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_0", FunctionType_CPU);
    auto& root = builder.subject().root();

    types::Scalar void_type(types::PrimitiveType::Void);
    types::Scalar int8_type(types::PrimitiveType::Int8);
    types::Pointer void_pointer;
    types::Pointer int8_pointer(int8_type);
    types::Pointer int8_pointer_pointer(*int8_pointer.clone());

    builder.set_return_type(void_pointer);
    builder.add_container("_this", void_pointer, true);
    builder.add_container("Apsi", void_pointer, true);
    builder.add_container("tpsi", void_pointer, true);
    builder.add_container("interfaceBouCoeffs", void_pointer, true);
    builder.add_container("interfaces", void_pointer, true);
    builder.add_container("cmpt", int8_type, true);
    builder.add_container("tt_meta", void_pointer);
    builder.add_container("k", void_pointer);
    builder.add_container("_ZN2tt5daisy4foam15ldu_tt_meta_mapE", int8_pointer_pointer, false, true);

    // Declare tt::daisy::foam::require_kernel_launcher()
    //         _ZN2tt5daisy4foam23require_kernel_launcherEv
    types::Function _ZN2tt5daisy4foam23require_kernel_launcherEv_type(void_pointer, false);
    builder.add_container("_ZN2tt5daisy4foam23require_kernel_launcherEv", _ZN2tt5daisy4foam23require_kernel_launcherEv_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Outputs
        auto& k = builder.add_access(block, "k");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam23require_kernel_launcherEv", {"_ret"}, {});
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", k, {}, void_pointer);
    }

    // Declare tt::daisy::tt_ldu_meta& tt::daisy::get_tt_meta<tt::daisy::tt_ldu_meta>(void const*, std::unordered_map<void const*, tt::daisy::tt_ldu_meta, std::hash<void const*>, std::equal_to<void const*>, std::allocator<std::pair<void const* const, tt::daisy::tt_ldu_meta> > >&)
    //         _ZN2tt5daisy11get_tt_metaINS0_11tt_ldu_metaEEERT_PKvRSt13unordered_mapIS6_S3_St4hashIS6_ESt8equal_toIS6_ESaISt4pairIKS6_S3_EEE
    types::Function _ZN2tt5daisy11get_tt_metaINS0_11tt_ldu_metaEEERT_PKvRSt13unordered_mapIS6_S3_St4hashIS6_ESt8equal_toIS6_ESaISt4pairIKS6_S3_EEE_type(void_pointer, false);
    _ZN2tt5daisy11get_tt_metaINS0_11tt_ldu_metaEEERT_PKvRSt13unordered_mapIS6_S3_St4hashIS6_ESt8equal_toIS6_ESaISt4pairIKS6_S3_EEE_type.add_param(void_pointer);
    _ZN2tt5daisy11get_tt_metaINS0_11tt_ldu_metaEEERT_PKvRSt13unordered_mapIS6_S3_St4hashIS6_ESt8equal_toIS6_ESaISt4pairIKS6_S3_EEE_type.add_param(void_pointer);
    builder.add_container("_ZN2tt5daisy11get_tt_metaINS0_11tt_ldu_metaEEERT_PKvRSt13unordered_mapIS6_S3_St4hashIS6_ESt8equal_toIS6_ESaISt4pairIKS6_S3_EEE", _ZN2tt5daisy11get_tt_metaINS0_11tt_ldu_metaEEERT_PKvRSt13unordered_mapIS6_S3_St4hashIS6_ESt8equal_toIS6_ESaISt4pairIKS6_S3_EEE_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& _this = builder.add_access(block, "_this");
        auto& ldu_tt_meta_map = builder.add_access(block, "_ZN2tt5daisy4foam15ldu_tt_meta_mapE");
        // Outputs
        auto& tt_meta = builder.add_access(block, "tt_meta");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy11get_tt_metaINS0_11tt_ldu_metaEEERT_PKvRSt13unordered_mapIS6_S3_St4hashIS6_ESt8equal_toIS6_ESaISt4pairIKS6_S3_EEE", {"_ret"}, {"_key", "_map"});
        // In edges
        builder.add_computational_memlet(block, _this, libnode, "_key", {}, void_pointer);
        builder.add_computational_memlet(block, ldu_tt_meta_map, libnode, "_map", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", tt_meta, {}, void_pointer);
    }

    // Declare tt::daisy::foam::copy_ldu_to_ellpack(tt::daisy::BufferPool&, tt::daisy::tt_ldu_meta&, Foam::lduMatrix const*)
    //         _ZN2tt5daisy4foam19copy_ldu_to_ellpackERNS0_10BufferPoolERNS0_11tt_ldu_metaEPKN4Foam9lduMatrixE
    types::Function _ZN2tt5daisy4foam19copy_ldu_to_ellpackERNS0_10BufferPoolERNS0_11tt_ldu_metaEPKN4Foam9lduMatrixE_type(void_type, false);
    _ZN2tt5daisy4foam19copy_ldu_to_ellpackERNS0_10BufferPoolERNS0_11tt_ldu_metaEPKN4Foam9lduMatrixE_type.add_param(void_pointer);
    _ZN2tt5daisy4foam19copy_ldu_to_ellpackERNS0_10BufferPoolERNS0_11tt_ldu_metaEPKN4Foam9lduMatrixE_type.add_param(void_pointer);
    _ZN2tt5daisy4foam19copy_ldu_to_ellpackERNS0_10BufferPoolERNS0_11tt_ldu_metaEPKN4Foam9lduMatrixE_type.add_param(void_pointer);
    builder.add_container("_ZN2tt5daisy4foam19copy_ldu_to_ellpackERNS0_10BufferPoolERNS0_11tt_ldu_metaEPKN4Foam9lduMatrixE", _ZN2tt5daisy4foam19copy_ldu_to_ellpackERNS0_10BufferPoolERNS0_11tt_ldu_metaEPKN4Foam9lduMatrixE_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& k = builder.add_access(block, "k");
        auto& tt_meta_in = builder.add_access(block, "tt_meta");
        auto& _this = builder.add_access(block, "_this");
        // Outputs
        auto& tt_meta_out = builder.add_access(block, "tt_meta");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam19copy_ldu_to_ellpackERNS0_10BufferPoolERNS0_11tt_ldu_metaEPKN4Foam9lduMatrixE", {"_tt_meta"}, {"_bufferPool", "_tt_meta", "_lduMat"});
        // In edges
        builder.add_computational_memlet(block, k, libnode, "_bufferPool", {}, void_pointer);
        builder.add_computational_memlet(block, tt_meta_in, libnode, "_tt_meta", {}, void_pointer);
        builder.add_computational_memlet(block, _this, libnode, "_lduMat", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "_tt_meta", tt_meta_out, {}, void_pointer);
    }

    builder.add_return(root, "tt_meta");

    return builder.move();
}

std::unique_ptr<StructuredSDFG> create_Amul_in_1() {
    builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_1", FunctionType_CPU);
    auto& root = builder.subject().root();

    types::Scalar void_type(types::PrimitiveType::Void);
    types::Scalar bool_type(types::PrimitiveType::Bool);
    types::Scalar int8_type(types::PrimitiveType::Int8);
    types::Scalar int32_type(types::PrimitiveType::Int32);
    types::Scalar int64_type(types::PrimitiveType::Int64);
    types::Pointer void_pointer;
    types::Pointer int8_pointer(int8_type);
    types::Pointer int64_pointer(int64_type);
    types::Pointer void_pointer_pointer(*void_pointer.clone());

    builder.set_return_type(void_pointer);
    builder.add_container("_this", void_pointer, true);
    builder.add_container("Apsi", void_pointer, true);
    builder.add_container("tpsi", void_pointer, true);
    builder.add_container("interfaceBouCoeffs", void_pointer, true);
    builder.add_container("interfaces", void_pointer, true);
    builder.add_container("cmpt", int8_type, true);
    builder.add_container("psi", void_pointer);
    builder.add_container("k", void_pointer);
    builder.add_container("tt_psi", void_pointer);

    // Declare tt::daisy::foam::require_kernel_launcher()
    //         _ZN2tt5daisy4foam23require_kernel_launcherEv
    types::Function _ZN2tt5daisy4foam23require_kernel_launcherEv_type(void_pointer, false);
    builder.add_container("_ZN2tt5daisy4foam23require_kernel_launcherEv", _ZN2tt5daisy4foam23require_kernel_launcherEv_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Outputs
        auto& k = builder.add_access(block, "k");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam23require_kernel_launcherEv", {"_ret"}, {});
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", k, {}, void_pointer);
    }

    // Declare Foam::tmp<Foam::Field<float> >::operator()() const
    //         _ZNK4Foam3tmpINS_5FieldIfEEEclEv
    types::Function _ZNK4Foam3tmpINS_5FieldIfEEEclEv_type(void_pointer, false);
    _ZNK4Foam3tmpINS_5FieldIfEEEclEv_type.add_param(void_pointer);
    builder.add_container("_ZNK4Foam3tmpINS_5FieldIfEEEclEv", _ZNK4Foam3tmpINS_5FieldIfEEEclEv_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& tpsi = builder.add_access(block, "tpsi");
        // Outputs
        auto& psi = builder.add_access(block, "psi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam3tmpINS_5FieldIfEEEclEv", {"_ret"}, {"__this"});
        // In edges
        builder.add_computational_memlet(block, tpsi, libnode, "__this", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", psi, {}, void_pointer);
    }

    // Declare tt::daisy::foam::copy_scalarField_to_device(tt::daisy::BufferPool&, Foam::Field<float> const&)
    //         _ZN2tt5daisy4foam26copy_scalarField_to_deviceERNS0_10BufferPoolERKN4Foam5FieldIfEE
    types::Function _ZN2tt5daisy4foam26copy_scalarField_to_deviceERNS0_10BufferPoolERKN4Foam5FieldIfEE_type(void_pointer, false);
    _ZN2tt5daisy4foam26copy_scalarField_to_deviceERNS0_10BufferPoolERKN4Foam5FieldIfEE_type.add_param(void_pointer);
    _ZN2tt5daisy4foam26copy_scalarField_to_deviceERNS0_10BufferPoolERKN4Foam5FieldIfEE_type.add_param(void_pointer);
    builder.add_container("_ZN2tt5daisy4foam26copy_scalarField_to_deviceERNS0_10BufferPoolERKN4Foam5FieldIfEE", _ZN2tt5daisy4foam26copy_scalarField_to_deviceERNS0_10BufferPoolERKN4Foam5FieldIfEE_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& k = builder.add_access(block, "k");
        auto& psi = builder.add_access(block, "psi");
        // Outputs
        auto& tt_psi = builder.add_access(block, "tt_psi");
        // CallNodes
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam26copy_scalarField_to_deviceERNS0_10BufferPoolERKN4Foam5FieldIfEE", {"_ret"}, {"_bufferPool", "_field"});
        // In edges
        builder.add_computational_memlet(block, k, libnode, "_bufferPool", {}, void_pointer);
        builder.add_computational_memlet(block, psi, libnode, "_field", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", tt_psi, {}, void_pointer);
    }

    builder.add_return(root, "tt_psi");

    return builder.move();
}

std::unique_ptr<StructuredSDFG> create_Amul_in_2() {
    builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_2", FunctionType_CPU);
    auto& root = builder.subject().root();

    types::Scalar int8_type(types::PrimitiveType::Int8);
    types::Scalar int64_type(types::PrimitiveType::Int64);
    types::Pointer void_pointer;
    types::Pointer int8_pointer(int8_type);
    types::Pointer int64_pointer(int64_type);
    types::Pointer void_pointer_pointer(*void_pointer.clone());

    builder.set_return_type(void_pointer);
    builder.add_container("_this", void_pointer, true);
    builder.add_container("Apsi", void_pointer, true);
    builder.add_container("tpsi", void_pointer, true);
    builder.add_container("interfaceBouCoeffs", void_pointer, true);
    builder.add_container("interfaces", void_pointer, true);
    builder.add_container("cmpt", int8_type, true);
    builder.add_container("k", void_pointer);
    builder.add_container("psi", void_pointer);
    builder.add_container("tt_Apsi", void_pointer);

    // Declare tt::daisy::foam::require_kernel_launcher()
    //         _ZN2tt5daisy4foam23require_kernel_launcherEv
    types::Function _ZN2tt5daisy4foam23require_kernel_launcherEv_type(void_pointer, false);
    builder.add_container("_ZN2tt5daisy4foam23require_kernel_launcherEv", _ZN2tt5daisy4foam23require_kernel_launcherEv_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Outputs
        auto& k = builder.add_access(block, "k");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam23require_kernel_launcherEv", {"_ret"}, {});
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", k, {}, void_pointer);
    }

    // Declare Foam::tmp<Foam::Field<float> >::operator()() const
    //         _ZNK4Foam3tmpINS_5FieldIfEEEclEv
    types::Function _ZNK4Foam3tmpINS_5FieldIfEEEclEv_type(void_pointer, false);
    _ZNK4Foam3tmpINS_5FieldIfEEEclEv_type.add_param(void_pointer);
    builder.add_container("_ZNK4Foam3tmpINS_5FieldIfEEEclEv", _ZNK4Foam3tmpINS_5FieldIfEEEclEv_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& tpsi = builder.add_access(block, "tpsi");
        // Outputs
        auto& psi = builder.add_access(block, "psi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam3tmpINS_5FieldIfEEEclEv", {"_ret"}, {"__this"});
        // In edges
        builder.add_computational_memlet(block, tpsi, libnode, "__this", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", psi, {}, void_pointer);
    }

    // Declare tt::daisy::foam::allocate_field_buffer(tt::daisy::BufferPool&, Foam::Field<float> const&)
    //         _ZN2tt5daisy4foam21allocate_field_bufferERNS0_10BufferPoolERKN4Foam5FieldIfEE
    types::Function _ZN2tt5daisy4foam21allocate_field_bufferERNS0_10BufferPoolERKN4Foam5FieldIfEE_type(void_pointer, false);
    _ZN2tt5daisy4foam21allocate_field_bufferERNS0_10BufferPoolERKN4Foam5FieldIfEE_type.add_param(void_pointer);
    _ZN2tt5daisy4foam21allocate_field_bufferERNS0_10BufferPoolERKN4Foam5FieldIfEE_type.add_param(void_pointer);
    builder.add_container("_ZN2tt5daisy4foam21allocate_field_bufferERNS0_10BufferPoolERKN4Foam5FieldIfEE", _ZN2tt5daisy4foam21allocate_field_bufferERNS0_10BufferPoolERKN4Foam5FieldIfEE_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& k = builder.add_access(block, "k");
        auto& psi = builder.add_access(block, "psi");
        // Outputs
        auto& tt_Apsi = builder.add_access(block, "tt_Apsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam21allocate_field_bufferERNS0_10BufferPoolERKN4Foam5FieldIfEE", {"_ret"}, {"_bufferPool", "_field"});
        // In edges
        builder.add_computational_memlet(block, k, libnode, "_bufferPool", {}, void_pointer);
        builder.add_computational_memlet(block, psi, libnode, "_field", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", tt_Apsi, {}, void_pointer);
    }

    builder.add_return(root, "tt_Apsi");

    return builder.move();
}

std::unique_ptr<StructuredSDFG> create_Amul_kernel() {
    builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_kernel", FunctionType_CPU);
    auto& root = builder.subject().root();

    types::Scalar void_type(types::PrimitiveType::Void);
    types::Scalar int8_type(types::PrimitiveType::Int8);
    types::Pointer void_pointer;

    builder.add_container("_this", void_pointer, true);
    builder.add_container("Apsi", void_pointer, true);
    builder.add_container("tpsi", void_pointer, true);
    builder.add_container("interfaceBouCoeffs", void_pointer, true);
    builder.add_container("interfaces", void_pointer, true);
    builder.add_container("cmpt", int8_type, true);
    builder.add_container("tt_meta", void_pointer, true);
    builder.add_container("tt_psi", void_pointer, true);
    builder.add_container("tt_Apsi", void_pointer, true);
    builder.add_container("k", void_pointer);

    // Declare tt::daisy::foam::require_kernel_launcher()
    //         _ZN2tt5daisy4foam23require_kernel_launcherEv
    types::Function _ZN2tt5daisy4foam23require_kernel_launcherEv_type(void_pointer, false);
    builder.add_container("_ZN2tt5daisy4foam23require_kernel_launcherEv", _ZN2tt5daisy4foam23require_kernel_launcherEv_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Outputs
        auto& k = builder.add_access(block, "k");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam23require_kernel_launcherEv", {"_ret"}, {});
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", k, {}, void_pointer);
    }

    // Declare tt::daisy::foam::tt_compute_amul(tt::daisy::foam::KernelLauncher&, tt::daisy::tt_ldu_meta&, tt::daisy::ReusableTtBuffer&, tt::daisy::ReusableTtBuffer&)
    //         _ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7_
    types::Function _ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7__type(void_type, false);
    _ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7__type.add_param(void_pointer);
    _ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7__type.add_param(void_pointer);
    _ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7__type.add_param(void_pointer);
    _ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7__type.add_param(void_pointer);
    builder.add_container("_ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7_", _ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7__type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& k_in = builder.add_access(block, "k");
        auto& tt_meta_in = builder.add_access(block, "tt_meta");
        auto& tt_psi_in = builder.add_access(block, "tt_psi");
        auto& tt_Apsi_in = builder.add_access(block, "tt_Apsi");
        // Outputs
        auto& k_out = builder.add_access(block, "k");
        auto& tt_meta_out = builder.add_access(block, "tt_meta");
        auto& tt_psi_out = builder.add_access(block, "tt_psi");
        auto& tt_Apsi_out = builder.add_access(block, "tt_Apsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam15tt_compute_amulERNS1_14KernelLauncherERNS0_11tt_ldu_metaERNS0_16ReusableTtBufferES7_", {"_k", "_tt_meta", "_tt_psi", "_tt_Apsi"}, {"_k", "_tt_meta", "_tt_psi", "_tt_Apsi"});
        // In edges
        builder.add_computational_memlet(block, k_in, libnode, "_k", {}, void_pointer);
        builder.add_computational_memlet(block, tt_meta_in, libnode, "_tt_meta", {}, void_pointer);
        builder.add_computational_memlet(block, tt_psi_in, libnode, "_tt_psi", {}, void_pointer);
        builder.add_computational_memlet(block, tt_Apsi_in, libnode, "_tt_Apsi", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "_k", k_out, {}, void_pointer);
        builder.add_computational_memlet(block, libnode, "_tt_meta", tt_meta_out, {}, void_pointer);
        builder.add_computational_memlet(block, libnode, "_tt_psi", tt_psi_out, {}, void_pointer);
        builder.add_computational_memlet(block, libnode, "_tt_Apsi", tt_Apsi_out, {}, void_pointer);
    }

    return builder.move();
}

std::unique_ptr<StructuredSDFG> create_Amul_out_0() {
    builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_0", FunctionType_CPU);
    auto& root = builder.subject().root();

    types::Scalar int8_type(types::PrimitiveType::Int8);
    types::Pointer void_pointer;

    builder.add_container("_this", void_pointer, true);
    builder.add_container("Apsi", void_pointer, true);
    builder.add_container("tpsi", void_pointer, true);
    builder.add_container("interfaceBouCoeffs", void_pointer, true);
    builder.add_container("interfaces", void_pointer, true);
    builder.add_container("cmpt", int8_type, true);
    builder.add_container("tt_meta", void_pointer, true);

    return builder.move();
}

std::unique_ptr<StructuredSDFG> create_Amul_out_1() {
    builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_1", FunctionType_CPU);
    auto& root = builder.subject().root();

    types::Scalar void_type(types::PrimitiveType::Void);
    types::Scalar int8_type(types::PrimitiveType::Int8);
    types::Pointer void_pointer;

    builder.add_container("_this", void_pointer, true);
    builder.add_container("Apsi", void_pointer, true);
    builder.add_container("tpsi", void_pointer, true);
    builder.add_container("interfaceBouCoeffs", void_pointer, true);
    builder.add_container("interfaces", void_pointer, true);
    builder.add_container("cmpt", int8_type, true);
    builder.add_container("tt_psi", void_pointer, true);
    builder.add_container("k", void_pointer);

    // Declare tt::daisy::foam::require_kernel_launcher()
    //         _ZN2tt5daisy4foam23require_kernel_launcherEv
    types::Function _ZN2tt5daisy4foam23require_kernel_launcherEv_type(void_pointer, false);
    builder.add_container("_ZN2tt5daisy4foam23require_kernel_launcherEv", _ZN2tt5daisy4foam23require_kernel_launcherEv_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Outputs
        auto& k = builder.add_access(block, "k");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam23require_kernel_launcherEv", {"_ret"}, {});
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", k, {}, void_pointer);
    }

    // Declare tt::daisy::BufferPool::freeBuffer(tt::daisy::ReusableTtBuffer&)
    //         _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE
    types::Function _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE_type(void_type, false);
    _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE_type.add_param(void_pointer);
    _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE_type.add_param(void_pointer);
    builder.add_container("_ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE", _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& k_in = builder.add_access(block, "k");
        auto& tt_psi_in = builder.add_access(block, "tt_psi");
        // Output
        auto& k_out = builder.add_access(block, "k");
        auto& tt_psi_out = builder.add_access(block, "tt_psi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE", {"__this", "_buffer"}, {"__this", "_buffer"});
        // In edges
        builder.add_computational_memlet(block, k_in, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, tt_psi_in, libnode, "_buffer", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "__this", k_out, {}, void_pointer);
        builder.add_computational_memlet(block, libnode, "_buffer", tt_psi_out, {}, void_pointer);
    }

    // Declare tt::daisy::foam::clear_tmp_field(Foam::tmp<Foam::Field<float> > const&)
    //         _ZN2tt5daisy4foam15clear_tmp_fieldERKN4Foam3tmpINS2_5FieldIfEEEE
    types::Function _ZN2tt5daisy4foam15clear_tmp_fieldERKN4Foam3tmpINS2_5FieldIfEEEE_type(void_type, false);
    _ZN2tt5daisy4foam15clear_tmp_fieldERKN4Foam3tmpINS2_5FieldIfEEEE_type.add_param(void_pointer);
    builder.add_container("_ZN2tt5daisy4foam15clear_tmp_fieldERKN4Foam3tmpINS2_5FieldIfEEEE", _ZN2tt5daisy4foam15clear_tmp_fieldERKN4Foam3tmpINS2_5FieldIfEEEE_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& tpsi = builder.add_access(block, "tpsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam15clear_tmp_fieldERKN4Foam3tmpINS2_5FieldIfEEEE", {}, {"_tfield"});
        // In edges
        builder.add_computational_memlet(block, tpsi, libnode, "_tfield", {}, void_pointer);
    }

    return builder.move();
}

std::unique_ptr<StructuredSDFG> create_Amul_out_2() {
    builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_2", FunctionType_CPU);
    auto& root = builder.subject().root();

    types::Scalar void_type(types::PrimitiveType::Void);
    types::Scalar int8_type(types::PrimitiveType::Int8);
    types::Scalar int32_type(types::PrimitiveType::Int32);
    types::Pointer void_pointer;

    builder.add_container("_this", void_pointer, true);
    builder.add_container("Apsi", void_pointer, true);
    builder.add_container("tpsi", void_pointer, true);
    builder.add_container("interfaceBouCoeffs", void_pointer, true);
    builder.add_container("interfaces", void_pointer, true);
    builder.add_container("cmpt", int8_type, true);
    builder.add_container("tt_Apsi", void_pointer, true);
    builder.add_container("k", void_pointer);

    // Declare tt::daisy::foam::require_kernel_launcher()
    //         _ZN2tt5daisy4foam23require_kernel_launcherEv
    types::Function _ZN2tt5daisy4foam23require_kernel_launcherEv_type(void_pointer, false);
    builder.add_container("_ZN2tt5daisy4foam23require_kernel_launcherEv", _ZN2tt5daisy4foam23require_kernel_launcherEv_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Outputs
        auto& k = builder.add_access(block, "k");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam23require_kernel_launcherEv", {"_ret"}, {});
        // Out edges
        builder.add_computational_memlet(block, libnode, "_ret", k, {}, void_pointer);
    }

    // Declare tt::daisy::foam::copy_scalarField_from_device(tt::daisy::BufferPool&, tt::daisy::ReusableTtBuffer&, Foam::Field<float>*, unsigned int)
    //         _ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj
    types::Function _ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj_type(void_type, false);
    _ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj_type.add_param(void_pointer);
    _ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj_type.add_param(void_pointer);
    _ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj_type.add_param(void_pointer);
    _ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj_type.add_param(int32_type);
    builder.add_container("_ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj", _ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& k_in = builder.add_access(block, "k");
        auto& tt_Apsi_in = builder.add_access(block, "tt_Apsi");
        auto& Apsi_in = builder.add_access(block, "Apsi");
        auto& constant_0 = builder.add_constant(block, "0", int32_type);
        // Outputs
        auto& k_out = builder.add_access(block, "k");
        auto& tt_Apsi_out = builder.add_access(block, "tt_Apsi");
        auto& Apsi_out = builder.add_access(block, "Apsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy4foam28copy_scalarField_from_deviceERNS0_10BufferPoolERNS0_16ReusableTtBufferEPN4Foam5FieldIfEEj", {"_bufferPool", "_buffer", "_field"}, {"_bufferPool", "_buffer", "_field", "_buf_offset"});
        // In edges
        builder.add_computational_memlet(block, k_in, libnode, "_bufferPool", {}, void_pointer);
        builder.add_computational_memlet(block, tt_Apsi_in, libnode, "_buffer", {}, void_pointer);
        builder.add_computational_memlet(block, Apsi_in, libnode, "_field", {}, void_pointer);
        builder.add_computational_memlet(block, constant_0, libnode, "_buf_offset", {}, int32_type);
        // Out edges
        builder.add_computational_memlet(block, libnode, "_bufferPool", k_out, {}, void_pointer);
        builder.add_computational_memlet(block, libnode, "_buffer", tt_Apsi_out, {}, void_pointer);
        builder.add_computational_memlet(block, libnode, "_field", Apsi_out, {}, void_pointer);
    }

    // Declare tt::daisy::BufferPool::freeBuffer(tt::daisy::ReusableTtBuffer&)
    //         _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE
    types::Function _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE_type(void_type, false);
    _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE_type.add_param(void_pointer);
    _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE_type.add_param(void_pointer);
    builder.add_container("_ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE", _ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& k_in = builder.add_access(block, "k");
        auto& tt_Apsi_in = builder.add_access(block, "tt_Apsi");
        // Output
        auto& k_out = builder.add_access(block, "k");
        auto& tt_Apsi_out = builder.add_access(block, "tt_Apsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZN2tt5daisy10BufferPool10freeBufferERNS0_16ReusableTtBufferE", {"__this", "_buffer"}, {"__this", "_buffer"});
        // In edges
        builder.add_computational_memlet(block, k_in, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, tt_Apsi_in, libnode, "_buffer", {}, void_pointer);
        // Out edges
        builder.add_computational_memlet(block, libnode, "__this", k_out, {}, void_pointer);
        builder.add_computational_memlet(block, libnode, "_buffer", tt_Apsi_out, {}, void_pointer);
    }

    return builder.move();
}

std::unique_ptr<StructuredSDFG> create_Amul_wrapper() {
    builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh", FunctionType_CPU);
    auto& root = builder.subject().root();

    types::Scalar void_type(types::PrimitiveType::Void);
    types::Scalar int8_type(types::PrimitiveType::Int8);
    types::Pointer void_pointer;

    builder.add_container("_this", void_pointer, true);
    builder.add_container("Apsi", void_pointer, true);
    builder.add_container("tpsi", void_pointer, true);
    builder.add_container("interfaceBouCoeffs", void_pointer, true);
    builder.add_container("interfaces", void_pointer, true);
    builder.add_container("cmpt", int8_type, true);
    builder.add_container("tt_meta", void_pointer);
    builder.add_container("tt_psi", void_pointer);
    builder.add_container("tt_Apsi", void_pointer);

    // Declare copy in
    types::Function copy_in_type(void_pointer, false);
    copy_in_type.add_param(void_pointer);
    copy_in_type.add_param(void_pointer);
    copy_in_type.add_param(void_pointer);
    copy_in_type.add_param(void_pointer);
    copy_in_type.add_param(void_pointer);
    copy_in_type.add_param(int8_type);
    builder.add_container("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_0", copy_in_type, false, true);
    builder.add_container("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_1", copy_in_type, false, true);
    builder.add_container("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_2", copy_in_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& _this = builder.add_access(block, "_this");
        auto& Apsi = builder.add_access(block, "Apsi");
        auto& tpsi = builder.add_access(block, "tpsi");
        auto& interfaceBouCoeffs = builder.add_access(block, "interfaceBouCoeffs");
        auto& interfaces = builder.add_access(block, "interfaces");
        auto& cmpt = builder.add_access(block, "cmpt");
        // Outputs
        auto& tt_meta = builder.add_access(block, "tt_meta");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_0", {"_ret"}, {"__this", "_Apsi", "_tpsi", "_interfaceBouCoeffs", "_interfaces", "_cmpt"});
        // In edge
        builder.add_computational_memlet(block, _this, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, Apsi, libnode, "_Apsi", {}, void_pointer);
        builder.add_computational_memlet(block, tpsi, libnode, "_tpsi", {}, void_pointer);
        builder.add_computational_memlet(block, interfaceBouCoeffs, libnode, "_interfaceBouCoeffs", {}, void_pointer);
        builder.add_computational_memlet(block, interfaces, libnode, "_interfaces", {}, void_pointer);
        builder.add_computational_memlet(block, cmpt, libnode, "_cmpt", {}, int8_type);
        // Out edge
        builder.add_computational_memlet(block, libnode, "_ret", tt_meta, {}, void_pointer);
    }

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& _this = builder.add_access(block, "_this");
        auto& Apsi = builder.add_access(block, "Apsi");
        auto& tpsi = builder.add_access(block, "tpsi");
        auto& interfaceBouCoeffs = builder.add_access(block, "interfaceBouCoeffs");
        auto& interfaces = builder.add_access(block, "interfaces");
        auto& cmpt = builder.add_access(block, "cmpt");
        // Outputs
        auto& tt_psi = builder.add_access(block, "tt_psi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_1", {"_ret"}, {"__this", "_Apsi", "_tpsi", "_interfaceBouCoeffs", "_interfaces", "_cmpt"});
        // In edge
        builder.add_computational_memlet(block, _this, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, Apsi, libnode, "_Apsi", {}, void_pointer);
        builder.add_computational_memlet(block, tpsi, libnode, "_tpsi", {}, void_pointer);
        builder.add_computational_memlet(block, interfaceBouCoeffs, libnode, "_interfaceBouCoeffs", {}, void_pointer);
        builder.add_computational_memlet(block, interfaces, libnode, "_interfaces", {}, void_pointer);
        builder.add_computational_memlet(block, cmpt, libnode, "_cmpt", {}, int8_type);
        // Out edge
        builder.add_computational_memlet(block, libnode, "_ret", tt_psi, {}, void_pointer);
    }

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& _this = builder.add_access(block, "_this");
        auto& Apsi = builder.add_access(block, "Apsi");
        auto& tpsi = builder.add_access(block, "tpsi");
        auto& interfaceBouCoeffs = builder.add_access(block, "interfaceBouCoeffs");
        auto& interfaces = builder.add_access(block, "interfaces");
        auto& cmpt = builder.add_access(block, "cmpt");
        // Outputs
        auto& tt_Apsi = builder.add_access(block, "tt_Apsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_in_2", {"_ret"}, {"__this", "_Apsi", "_tpsi", "_interfaceBouCoeffs", "_interfaces", "_cmpt"});
        // In edge
        builder.add_computational_memlet(block, _this, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, Apsi, libnode, "_Apsi", {}, void_pointer);
        builder.add_computational_memlet(block, tpsi, libnode, "_tpsi", {}, void_pointer);
        builder.add_computational_memlet(block, interfaceBouCoeffs, libnode, "_interfaceBouCoeffs", {}, void_pointer);
        builder.add_computational_memlet(block, interfaces, libnode, "_interfaces", {}, void_pointer);
        builder.add_computational_memlet(block, cmpt, libnode, "_cmpt", {}, int8_type);
        // Out edge
        builder.add_computational_memlet(block, libnode, "_ret", tt_Apsi, {}, void_pointer);
    }

    // Declare kernel
    types::Function kernel_type(void_type, false);
    kernel_type.add_param(void_pointer);
    kernel_type.add_param(void_pointer);
    kernel_type.add_param(void_pointer);
    kernel_type.add_param(void_pointer);
    kernel_type.add_param(void_pointer);
    kernel_type.add_param(int8_type);
    kernel_type.add_param(void_pointer);
    kernel_type.add_param(void_pointer);
    kernel_type.add_param(void_pointer);
    builder.add_container("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_kernel", kernel_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& _this = builder.add_access(block, "_this");
        auto& Apsi = builder.add_access(block, "Apsi");
        auto& tpsi = builder.add_access(block, "tpsi");
        auto& interfaceBouCoeffs = builder.add_access(block, "interfaceBouCoeffs");
        auto& interfaces = builder.add_access(block, "interfaces");
        auto& cmpt = builder.add_access(block, "cmpt");
        auto& tt_meta = builder.add_access(block, "tt_meta");
        auto& tt_psi = builder.add_access(block, "tt_psi");
        auto& tt_Apsi_in = builder.add_access(block, "tt_Apsi");
        // Outpus
        auto& tt_Apsi_out = builder.add_access(block, "tt_Apsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_kernel", {"_arg2"}, {"__this", "_Apsi", "_tpsi", "_interfaceBouCoeffs", "_interfaces", "_cmpt", "_tt_meta", "_tt_psi", "_tt_Apsi"});
        // In edge
        builder.add_computational_memlet(block, _this, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, Apsi, libnode, "_Apsi", {}, void_pointer);
        builder.add_computational_memlet(block, tpsi, libnode, "_tpsi", {}, void_pointer);
        builder.add_computational_memlet(block, interfaceBouCoeffs, libnode, "_interfaceBouCoeffs", {}, void_pointer);
        builder.add_computational_memlet(block, interfaces, libnode, "_interfaces", {}, void_pointer);
        builder.add_computational_memlet(block, cmpt, libnode, "_cmpt", {}, int8_type);
        builder.add_computational_memlet(block, tt_meta, libnode, "_tt_meta", {}, void_pointer);
        builder.add_computational_memlet(block, tt_psi, libnode, "_tt_psi", {}, void_pointer);
        builder.add_computational_memlet(block, tt_Apsi_in, libnode, "_tt_Apsi", {}, void_pointer);
        // Out edge
        builder.add_computational_memlet(block, libnode, "_arg2", tt_Apsi_out, {}, void_pointer);
    }

    // Declare copy out
    types::Function copy_out_type(void_type, false);
    copy_out_type.add_param(void_pointer);
    copy_out_type.add_param(void_pointer);
    copy_out_type.add_param(void_pointer);
    copy_out_type.add_param(void_pointer);
    copy_out_type.add_param(void_pointer);
    copy_out_type.add_param(int8_type);
    copy_out_type.add_param(void_pointer);
    builder.add_container("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_0", copy_out_type, false, true);
    builder.add_container("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_1", copy_out_type, false, true);
    builder.add_container("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_2", copy_out_type, false, true);

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& _this = builder.add_access(block, "_this");
        auto& Apsi = builder.add_access(block, "Apsi");
        auto& tpsi = builder.add_access(block, "tpsi");
        auto& interfaceBouCoeffs = builder.add_access(block, "interfaceBouCoeffs");
        auto& interfaces = builder.add_access(block, "interfaces");
        auto& cmpt = builder.add_access(block, "cmpt");
        auto& tt_Apsi = builder.add_access(block, "tt_Apsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_0", {}, {"__this", "_Apsi", "_tpsi", "_interfaceBouCoeffs", "_interfaces", "_cmpt", "_tt_meta"});
        // In edge
        builder.add_computational_memlet(block, _this, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, Apsi, libnode, "_Apsi", {}, void_pointer);
        builder.add_computational_memlet(block, tpsi, libnode, "_tpsi", {}, void_pointer);
        builder.add_computational_memlet(block, interfaceBouCoeffs, libnode, "_interfaceBouCoeffs", {}, void_pointer);
        builder.add_computational_memlet(block, interfaces, libnode, "_interfaces", {}, void_pointer);
        builder.add_computational_memlet(block, cmpt, libnode, "_cmpt", {}, int8_type);
        builder.add_computational_memlet(block, tt_Apsi, libnode, "_tt_meta", {}, void_pointer);
    }

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& _this = builder.add_access(block, "_this");
        auto& Apsi = builder.add_access(block, "Apsi");
        auto& tpsi = builder.add_access(block, "tpsi");
        auto& interfaceBouCoeffs = builder.add_access(block, "interfaceBouCoeffs");
        auto& interfaces = builder.add_access(block, "interfaces");
        auto& cmpt = builder.add_access(block, "cmpt");
        auto& tt_psi = builder.add_access(block, "tt_psi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_1", {}, {"__this", "_Apsi", "_tpsi", "_interfaceBouCoeffs", "_interfaces", "_cmpt", "_tt_psi"});
        // In edge
        builder.add_computational_memlet(block, _this, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, Apsi, libnode, "_Apsi", {}, void_pointer);
        builder.add_computational_memlet(block, tpsi, libnode, "_tpsi", {}, void_pointer);
        builder.add_computational_memlet(block, interfaceBouCoeffs, libnode, "_interfaceBouCoeffs", {}, void_pointer);
        builder.add_computational_memlet(block, interfaces, libnode, "_interfaces", {}, void_pointer);
        builder.add_computational_memlet(block, cmpt, libnode, "_cmpt", {}, int8_type);
        builder.add_computational_memlet(block, tt_psi, libnode, "_tt_psi", {}, void_pointer);
    }

    {
        auto& block = builder.add_block(root);
        // Inputs
        auto& _this = builder.add_access(block, "_this");
        auto& Apsi = builder.add_access(block, "Apsi");
        auto& tpsi = builder.add_access(block, "tpsi");
        auto& interfaceBouCoeffs = builder.add_access(block, "interfaceBouCoeffs");
        auto& interfaces = builder.add_access(block, "interfaces");
        auto& cmpt = builder.add_access(block, "cmpt");
        auto& tt_Apsi = builder.add_access(block, "tt_Apsi");
        // CallNode
        auto& libnode = builder.add_library_node<data_flow::CallNode, const std::string&, const std::vector<std::string>&, const std::vector<std::string>&>(block, DebugInfo(), "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh_out_2", {}, {"__this", "_Apsi", "_tpsi", "_interfaceBouCoeffs", "_interfaces", "_cmpt", "_tt_Apsi"});
        // In edge
        builder.add_computational_memlet(block, _this, libnode, "__this", {}, void_pointer);
        builder.add_computational_memlet(block, Apsi, libnode, "_Apsi", {}, void_pointer);
        builder.add_computational_memlet(block, tpsi, libnode, "_tpsi", {}, void_pointer);
        builder.add_computational_memlet(block, interfaceBouCoeffs, libnode, "_interfaceBouCoeffs", {}, void_pointer);
        builder.add_computational_memlet(block, interfaces, libnode, "_interfaces", {}, void_pointer);
        builder.add_computational_memlet(block, cmpt, libnode, "_cmpt", {}, int8_type);
        builder.add_computational_memlet(block, tt_Apsi, libnode, "_tt_Apsi", {}, void_pointer);
    }

    return builder.move();
}

std::list<std::unique_ptr<StructuredSDFG>> create_Amul() {
    std::list<std::unique_ptr<StructuredSDFG>> result;
    result.push_back(create_Amul_in_0());
    result.push_back(create_Amul_in_1());
    result.push_back(create_Amul_in_2());
    result.push_back(create_Amul_kernel());
    result.push_back(create_Amul_out_0());
    result.push_back(create_Amul_out_1());
    result.push_back(create_Amul_out_2());
    result.push_back(create_Amul_wrapper());
    return result;
}

extern "C" plugins::Plugin register_docc_plugin() {
    return {
        .name = "OpenFOAM_DOCCPlugin",
        .version = "0.0.1",
        .description = "A plugin that provides OpenFOAM kernels for docc",
        .register_plugin_callback = []() {},
        .sdfg_lookup = [](std::string name) {
            if (name == "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh") {
                return create_Amul();
            }
            return std::list<std::unique_ptr<StructuredSDFG>>();
        }
    };
}
