#ifndef COMPILEFORGE_GRAPH_DEPENDENCY_GRAPH_HPP
#define COMPILEFORGE_GRAPH_DEPENDENCY_GRAPH_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <compileforge/core/types.hpp>

namespace compileforge {

class DependencyGraph {
public:
    void add_node(const FileNode& node);
    void add_edge(const std::string& from_path, const std::string& to_path);

    [[nodiscard]] bool has_node(const std::string& path) const;
    [[nodiscard]] const FileNode* get_node(const std::string& path) const;
    [[nodiscard]] FileNode* get_node_mut(const std::string& path);

    [[nodiscard]] const std::vector<std::string>& get_outgoing_edges(const std::string& path) const;
    [[nodiscard]] const std::vector<std::string>& get_incoming_edges(const std::string& path) const;

    [[nodiscard]] std::vector<std::string> all_nodes() const;
    [[nodiscard]] size_t node_count() const { return nodes_.size(); }

    void compute_fan_stats();

    [[nodiscard]] std::unordered_set<std::string> get_transitive_dependencies(const std::string& path) const;
    [[nodiscard]] std::unordered_set<std::string> get_transitive_dependents(const std::string& path) const;

private:
    std::unordered_map<std::string, FileNode> nodes_;
    std::unordered_map<std::string, std::vector<std::string>> outgoing_edges_; // file -> dependencies
    std::unordered_map<std::string, std::vector<std::string>> incoming_edges_; // file -> dependents
};

} // namespace compileforge

#endif // COMPILEFORGE_GRAPH_DEPENDENCY_GRAPH_HPP
