#include <memory>
#include <sdfg/builder/structured_sdfg_builder.h>
#include <sdfg/codegen/dispatchers/node_dispatcher_registry.h>
#include <sdfg/function.h>
#include <sdfg/plugins/plugins.h>
#include <sdfg/serializer/json_serializer.h>
#include <sdfg/types/pointer.h>
#include <sdfg/types/scalar.h>
#include <sdfg/types/type.h>
#include <string>

#include "sdfg/structured_sdfg.h"

std::unique_ptr<sdfg::StructuredSDFG> create_Amul() {
    sdfg::builder::StructuredSDFGBuilder builder("_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh", sdfg::FunctionType_CPU);

    sdfg::types::Scalar int8_type(sdfg::types::PrimitiveType::Int8);
    sdfg::types::Pointer void_pointer;
    builder.add_container("_0", void_pointer, true);
    builder.add_container("_1", void_pointer, true);
    builder.add_container("_2", void_pointer, true);
    builder.add_container("_3", void_pointer, true);
    builder.add_container("_4", void_pointer, true);
    builder.add_container("_5", int8_type, true);

    return builder.move();
}

extern "C" sdfg::plugins::Plugin register_docc_plugin() {
    return {
        .name = "OpenFOAM_DOCCPlugin",
        .version = "0.0.1",
        .description = "A plugin that provides OpenFOAM kernels for docc",
        .register_plugin_callback = []() {},
        .sdfg_lookup = [](std::string name) {
            if (name == "_ZNK4Foam9lduMatrix4AmulERNS_5FieldIfEERKNS_3tmpIS2_EERKNS_10FieldFieldIS1_fEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEEh") {
                return create_Amul();
            }
            return std::unique_ptr<sdfg::StructuredSDFG>(nullptr);
        }
    };
}
