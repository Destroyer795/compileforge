#include <compileforge/cache/analysis_cache.hpp>
#include <compileforge/core/json.hpp>
#include <fstream>

namespace compileforge {

Result<AnalysisCache> AnalysisCache::load(const std::string& cache_filepath) {
    AnalysisCache cache;
    std::ifstream ifs(cache_filepath);
    if (!ifs) return cache; // Fresh cache

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto parse_res = JsonValue::parse(content);
    if (parse_res.is_error()) return cache;

    const auto& root = parse_res.value();
    if (!root.is_object()) return cache;

    for (const auto& [path, item] : root.as_object()) {
        CachedFileEntry entry;
        entry.content_hash = item["hash"].as_string();

        entry.metrics.total_lines = item["metrics"]["total_lines"].as_size_t();
        entry.metrics.sloc = item["metrics"]["sloc"].as_size_t();
        entry.metrics.comment_lines = item["metrics"]["comment_lines"].as_size_t();
        entry.metrics.blank_lines = item["metrics"]["blank_lines"].as_size_t();
        entry.metrics.cyclomatic_complexity = item["metrics"]["cyclomatic_complexity"].as_size_t();
        entry.metrics.macro_count = item["metrics"]["macro_count"].as_size_t();
        entry.metrics.has_pragma_once = item["metrics"]["has_pragma_once"].as_bool();
        entry.metrics.has_header_guard = item["metrics"]["has_header_guard"].as_bool();

        if (item["includes"].is_array()) {
            for (const auto& inc_val : item["includes"].as_array()) {
                IncludeDirective inc;
                inc.raw_path = inc_val["raw_path"].as_string();
                inc.kind = (inc_val["kind"].as_string() == "User") ? IncludeKind::User : IncludeKind::System;
                inc.line_number = inc_val["line"].as_size_t();
                entry.includes.push_back(inc);
            }
        }

        cache.entries_[path] = std::move(entry);
    }

    return cache;
}

Result<void> AnalysisCache::save(const std::string& cache_filepath) const {
    JsonValue::ObjectType root_obj;

    for (const auto& [path, entry] : entries_) {
        JsonValue::ObjectType item_obj;
        item_obj["hash"] = entry.content_hash;

        JsonValue::ObjectType m_obj;
        m_obj["total_lines"] = entry.metrics.total_lines;
        m_obj["sloc"] = entry.metrics.sloc;
        m_obj["comment_lines"] = entry.metrics.comment_lines;
        m_obj["blank_lines"] = entry.metrics.blank_lines;
        m_obj["cyclomatic_complexity"] = entry.metrics.cyclomatic_complexity;
        m_obj["macro_count"] = entry.metrics.macro_count;
        m_obj["has_pragma_once"] = entry.metrics.has_pragma_once;
        m_obj["has_header_guard"] = entry.metrics.has_header_guard;
        item_obj["metrics"] = m_obj;

        JsonValue::ArrayType inc_arr;
        for (const auto& inc : entry.includes) {
            JsonValue::ObjectType inc_obj;
            inc_obj["raw_path"] = inc.raw_path;
            inc_obj["kind"] = (inc.kind == IncludeKind::User) ? "User" : "System";
            inc_obj["line"] = inc.line_number;
            inc_arr.push_back(inc_obj);
        }
        item_obj["includes"] = inc_arr;

        root_obj[path] = item_obj;
    }

    JsonValue val(root_obj);
    std::ofstream ofs(cache_filepath);
    if (!ofs) {
        return Error{ErrorCode::IoError, "Could not open cache file for writing: " + cache_filepath};
    }
    ofs << val.serialize(2);
    return Result<void>();
}

const CachedFileEntry* AnalysisCache::get(const std::string& relative_path, const std::string& content_hash) const {
    auto it = entries_.find(relative_path);
    if (it != entries_.end() && it->second.content_hash == content_hash) {
        return &it->second;
    }
    return nullptr;
}

void AnalysisCache::put(
    const std::string& relative_path,
    const std::string& content_hash,
    SourceMetricsData metrics,
    std::vector<IncludeDirective> includes
) {
    CachedFileEntry entry;
    entry.content_hash = content_hash;
    entry.metrics = metrics;
    entry.includes = std::move(includes);
    entries_[relative_path] = std::move(entry);
}

} // namespace compileforge
