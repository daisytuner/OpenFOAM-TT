#include "simple_node.h"

namespace simple {

SimpleNode::SimpleNode(
    size_t element_id,
    const sdfg::DebugInfo& debug_info,
    const sdfg::graph::Vertex vertex,
    sdfg::data_flow::DataFlowGraph& parent
)
    : sdfg::data_flow::LibraryNode(
          element_id, debug_info, vertex, parent, LibraryNodeType_Simple, {"_ret"}, {"_in1", "_in2"}, false, sdfg::data_flow::ImplementationType_NONE
      ) {}

void SimpleNode::validate(const sdfg::Function& function) const {}

sdfg::symbolic::SymbolSet SimpleNode::symbols() const { return sdfg::symbolic::SymbolSet(); }

std::unique_ptr<sdfg::data_flow::DataFlowNode> SimpleNode::clone(size_t element_id, const sdfg::graph::Vertex vertex, sdfg::data_flow::DataFlowGraph& parent)
    const {
    return std::make_unique<SimpleNode>(element_id, debug_info_, vertex, parent);
}

void SimpleNode::replace(const sdfg::symbolic::Expression old_expression, const sdfg::symbolic::Expression new_expression) { return; }

nlohmann::json SimpleNodeSerializer::serialize(const sdfg::data_flow::LibraryNode& library_node) {
    const SimpleNode& node = static_cast<const SimpleNode&>(library_node);
    nlohmann::json j;

    j["code"] = node.code().value();
    j["outputs"] = node.outputs();
    j["inputs"] = node.inputs();
    j["side_effect"] = node.side_effect();

    return j;
}

sdfg::data_flow::LibraryNode& SimpleNodeSerializer::deserialize(
    const nlohmann::json& j, sdfg::builder::StructuredSDFGBuilder& builder, sdfg::structured_control_flow::Block& parent
) {
    assert(j.contains("element_id"));
    assert(j.contains("code"));
    assert(j.contains("outputs"));
    assert(j.contains("inputs"));
    assert(j.contains("debug_info"));

    auto code = j["code"].get<std::string>();
    if (code != LibraryNodeType_Simple.value()) {
        throw std::runtime_error("Invalid library node code");
    }

    // Extract debug info using JSONSerializer
    sdfg::serializer::JSONSerializer serializer;
    sdfg::DebugInfo debug_info = serializer.json_to_debug_info(j["debug_info"]);

    return builder.add_library_node<SimpleNode>(parent, debug_info);
}

SimpleDispatcher::SimpleDispatcher(
    sdfg::codegen::LanguageExtension& language_extension,
    const sdfg::Function& function,
    const sdfg::data_flow::DataFlowGraph& data_flow_graph,
    const sdfg::data_flow::LibraryNode& node
)
    : sdfg::codegen::LibraryNodeDispatcher(language_extension, function, data_flow_graph, node) {}

void SimpleDispatcher::dispatch(
    sdfg::codegen::PrettyPrinter& stream,
    sdfg::codegen::PrettyPrinter& globals_stream,
    sdfg::codegen::CodeSnippetFactory& library_snippet_factory
) {
    stream << "// Library Node: Simple Node" << std::endl;
    stream << node_.outputs().at(0) << " = " << node_.inputs().at(0) << " + " << node_.inputs().at(1) << ";";
}

} // namespace simple
