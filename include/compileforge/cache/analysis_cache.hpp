#ifndef COMPILEFORGE_CACHE_ANALYSIS_CACHE_HPP
#define COMPILEFORGE_CACHE_ANALYSIS_CACHE_HPP

#include <string>
#include <unordered_map>
#include "../core/result.hpp"
#include "../core/types.hpp"

namespace compileforge {

struct CachedFileEntry {
    std::string content_hash;
    SourceMetricsData metrics;
    std::vector<IncludeDirective> includes;
};

class AnalysisCache {
public:
    static Result<AnalysisCache> load(const std::string& cache_filepath);
    Result<void> save(const std::string& cache_filepath) const;

    [[nodiscard]] const CachedFileEntry* get(const std::string& relative_path, const std::string& content_hash) const;
    void put(const std::string& relative_path, const std::string& content_hash, SourceMetricsData metrics, std::vector<IncludeDirective> includes);

private:
    std::unordered_map<std::string, CachedFileEntry> entries_;
};

} // namespace compileforge

#endif // COMPILEFORGE_CACHE_ANALYSIS_CACHE_HPP
