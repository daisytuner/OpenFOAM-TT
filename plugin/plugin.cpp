#include <sdfg/codegen/dispatchers/node_dispatcher_registry.h>
#include <sdfg/plugins/plugins.h>
#include <sdfg/serializer/json_serializer.h>

#include "simple_node.h"

extern "C" sdfg::plugins::Plugin register_docc_plugin() {
    return {
        .name = "OpenFOAM_DOCCPlugin",
        .version = "0.0.1",
        .description = "A plugin that provides OpenFOAM kernels for docc",
        .register_plugin_callback = []() {
            sdfg::serializer::LibraryNodeSerializerRegistry::instance()
                .register_library_node_serializer(simple::LibraryNodeType_Simple.value(), []() {
                    return std::make_unique<simple::SimpleNodeSerializer>();
                });

            sdfg::codegen::LibraryNodeDispatcherRegistry::instance().register_library_node_dispatcher(
                simple::LibraryNodeType_Simple.value() + "::" + sdfg::data_flow::ImplementationType_NONE.value(),
                [](sdfg::codegen::LanguageExtension& language_extension,
                const sdfg::Function& function,
                const sdfg::data_flow::DataFlowGraph& data_flow_graph,
                const sdfg::data_flow::LibraryNode& node) {
                    return std::make_unique<simple::SimpleDispatcher>(language_extension, function, data_flow_graph, node);
                }
            );
        }
    };
}
