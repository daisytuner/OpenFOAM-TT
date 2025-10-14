#pragma once

#include <sdfg/codegen/dispatchers/block_dispatcher.h>
#include <sdfg/data_flow/library_node.h>
#include <sdfg/serializer/json_serializer.h>

namespace simple {

inline sdfg::data_flow::LibraryNodeCode LibraryNodeType_Simple("simple");

class SimpleNode : public sdfg::data_flow::LibraryNode {
public:
    SimpleNode(
        size_t element_id,
        const sdfg::DebugInfo& debug_info,
        const sdfg::graph::Vertex vertex,
        sdfg::data_flow::DataFlowGraph& parent
    );

    void validate(const sdfg::Function& function) const override;

    sdfg::symbolic::SymbolSet symbols() const override;

    std::unique_ptr<sdfg::data_flow::DataFlowNode> clone(size_t element_id, const sdfg::graph::Vertex vertex, sdfg::data_flow::DataFlowGraph& parent)
        const override;

    void replace(const sdfg::symbolic::Expression old_expression, const sdfg::symbolic::Expression new_expression) override;
};

class SimpleNodeSerializer : public sdfg::serializer::LibraryNodeSerializer {
public:
    nlohmann::json serialize(const sdfg::data_flow::LibraryNode& library_node) override;

    sdfg::data_flow::LibraryNode& deserialize(
        const nlohmann::json& j, sdfg::builder::StructuredSDFGBuilder& builder, sdfg::structured_control_flow::Block& parent
    ) override;
};

class SimpleDispatcher : public sdfg::codegen::LibraryNodeDispatcher {
public:
    SimpleDispatcher(
        sdfg::codegen::LanguageExtension& language_extension,
        const sdfg::Function& function,
        const sdfg::data_flow::DataFlowGraph& data_flow_graph,
        const sdfg::data_flow::LibraryNode& node
    );

    void dispatch(
        sdfg::codegen::PrettyPrinter& stream,
        sdfg::codegen::PrettyPrinter& globals_stream,
        sdfg::codegen::CodeSnippetFactory& library_snippet_factory
    ) override;
};

} // namespace simple
