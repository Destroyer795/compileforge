#include "../../include/compileforge/graph/dependency_graph.hpp"
#include <queue>
#include <algorithm>

namespace compileforge {

void DependencyGraph::add_node(const FileNode& node) {
    if (!has_node(node.relative_path)) {
        nodes_[node.relative_path] = node;
    }
}

void DependencyGraph::add_edge(const std::string& from_path, const std::string& to_path) {
    if (from_path.empty() || to_path.empty() || from_path == to_path) return;

    auto& out = outgoing_edges_[from_path];
    if (std::find(out.begin(), out.end(), to_path) == out.end()) {
        out.push_back(to_path);
    }

    auto& in = incoming_edges_[to_path];
    if (std::find(in.begin(), in.end(), from_path) == in.end()) {
        in.push_back(from_path);
    }
}

bool DependencyGraph::has_node(const std::string& path) const {
    return nodes_.find(path) != nodes_.end();
}

const FileNode* DependencyGraph::get_node(const std::string& path) const {
    auto it = nodes_.find(path);
    if (it != nodes_.end()) return &it->second;
    return nullptr;
}

FileNode* DependencyGraph::get_node_mut(const std::string& path) {
    auto it = nodes_.find(path);
    if (it != nodes_.end()) return &it->second;
    return nullptr;
}

const std::vector<std::string>& DependencyGraph::get_outgoing_edges(const std::string& path) const {
    static const std::vector<std::string> empty;
    auto it = outgoing_edges_.find(path);
    if (it != outgoing_edges_.end()) return it->second;
    return empty;
}

const std::vector<std::string>& DependencyGraph::get_incoming_edges(const std::string& path) const {
    static const std::vector<std::string> empty;
    auto it = incoming_edges_.find(path);
    if (it != incoming_edges_.end()) return it->second;
    return empty;
}

std::vector<std::string> DependencyGraph::all_nodes() const {
    std::vector<std::string> res;
    res.reserve(nodes_.size());
    for (const auto& [k, v] : nodes_) {
        res.push_back(k);
    }
    return res;
}

std::unordered_set<std::string> DependencyGraph::get_transitive_dependencies(const std::string& start_path) const {
    std::unordered_set<std::string> visited;
    std::queue<std::string> q;
    q.push(start_path);

    while (!q.empty()) {
        std::string curr = q.front();
        q.pop();

        for (const auto& dep : get_outgoing_edges(curr)) {
            if (visited.find(dep) == visited.end()) {
                visited.insert(dep);
                q.push(dep);
            }
        }
    }
    return visited;
}

std::unordered_set<std::string> DependencyGraph::get_transitive_dependents(const std::string& start_path) const {
    std::unordered_set<std::string> visited;
    std::queue<std::string> q;
    q.push(start_path);

    while (!q.empty()) {
        std::string curr = q.front();
        q.pop();

        for (const auto& dependent : get_incoming_edges(curr)) {
            if (visited.find(dependent) == visited.end()) {
                visited.insert(dependent);
                q.push(dependent);
            }
        }
    }
    return visited;
}

void DependencyGraph::compute_fan_stats() {
    for (auto& [path, node] : nodes_) {
        node.fan_stats.fan_out_direct = get_outgoing_edges(path).size();
        node.fan_stats.fan_in_direct = get_incoming_edges(path).size();

        auto transitive_deps = get_transitive_dependencies(path);
        node.fan_stats.fan_out_transitive = transitive_deps.size();

        auto transitive_dependents = get_transitive_dependents(path);
        node.fan_stats.fan_in_transitive = transitive_dependents.size();

        // Calculate impact score: fan_in_transitive * (LOC / 100)
        size_t loc = node.metrics.total_lines > 0 ? node.metrics.total_lines : 1;
        node.fan_stats.impact_score = node.fan_stats.fan_in_transitive * (1 + loc / 50);
    }
}

} // namespace compileforge
