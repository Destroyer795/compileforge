#include "../../include/compileforge/graph/cycle_detector.hpp"
#include <stack>
#include <algorithm>

namespace compileforge {

struct TarjanNode {
    int index{-1};
    int lowlink{-1};
    bool on_stack{false};
};

static void scc_dfs(
    const std::string& u,
    const DependencyGraph& graph,
    int& current_index,
    std::stack<std::string>& st,
    std::unordered_map<std::string, TarjanNode>& tnodes,
    std::vector<std::vector<std::string>>& sccs
) {
    auto& u_node = tnodes[u];
    u_node.index = current_index;
    u_node.lowlink = current_index;
    ++current_index;
    st.push(u);
    u_node.on_stack = true;

    for (const auto& v : graph.get_outgoing_edges(u)) {
        if (tnodes.find(v) == tnodes.end() || tnodes[v].index == -1) {
            scc_dfs(v, graph, current_index, st, tnodes, sccs);
            tnodes[u].lowlink = std::min(tnodes[u].lowlink, tnodes[v].lowlink);
        } else if (tnodes[v].on_stack) {
            tnodes[u].lowlink = std::min(tnodes[u].lowlink, tnodes[v].index);
        }
    }

    if (tnodes[u].lowlink == tnodes[u].index) {
        std::vector<std::string> scc;
        while (true) {
            std::string w = st.top();
            st.pop();
            tnodes[w].on_stack = false;
            scc.push_back(w);
            if (w == u) break;
        }
        if (scc.size() > 1) {
            sccs.push_back(scc);
        } else if (scc.size() == 1) {
            // Check self loop
            const auto& out = graph.get_outgoing_edges(scc[0]);
            if (std::find(out.begin(), out.end(), scc[0]) != out.end()) {
                sccs.push_back(scc);
            }
        }
    }
}

static bool find_path_dfs(
    const std::string& current,
    const std::string& target,
    const DependencyGraph& graph,
    const std::unordered_set<std::string>& scc_set,
    std::unordered_set<std::string>& visited,
    std::vector<std::string>& path
) {
    path.push_back(current);
    visited.insert(current);

    if (current == target && path.size() > 1) {
        return true;
    }

    for (const auto& next : graph.get_outgoing_edges(current)) {
        if (scc_set.find(next) == scc_set.end()) continue;
        if (next == target && path.size() >= 2) {
            path.push_back(target);
            return true;
        }
        if (visited.find(next) == visited.end()) {
            if (find_path_dfs(next, target, graph, scc_set, visited, path)) {
                return true;
            }
        }
    }

    path.pop_back();
    return false;
}

std::vector<DependencyCycle> CycleDetector::detect_cycles(const DependencyGraph& graph) {
    std::vector<std::vector<std::string>> sccs;
    std::stack<std::string> st;
    std::unordered_map<std::string, TarjanNode> tnodes;
    int current_index = 0;

    for (const auto& node_path : graph.all_nodes()) {
        if (tnodes.find(node_path) == tnodes.end() || tnodes[node_path].index == -1) {
            scc_dfs(node_path, graph, current_index, st, tnodes, sccs);
        }
    }

    std::vector<DependencyCycle> cycles;
    for (const auto& scc : sccs) {
        std::unordered_set<std::string> scc_set(scc.begin(), scc.end());
        std::string start_node = scc[0];

        std::unordered_set<std::string> visited;
        std::vector<std::string> path;

        if (find_path_dfs(start_node, start_node, graph, scc_set, visited, path)) {
            DependencyCycle cycle;
            cycle.cycle_path = path;
            cycle.length = path.size() > 0 ? path.size() - 1 : 0;
            cycles.push_back(std::move(cycle));
        } else {
            // Fallback cycle representation
            DependencyCycle cycle;
            cycle.cycle_path = scc;
            cycle.cycle_path.push_back(scc[0]);
            cycle.length = scc.size();
            cycles.push_back(std::move(cycle));
        }
    }

    return cycles;
}

} // namespace compileforge
