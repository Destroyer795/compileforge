#ifndef COMPILEFORGE_PARSER_INCLUDE_PARSER_HPP
#define COMPILEFORGE_PARSER_INCLUDE_PARSER_HPP

#include <string>
#include <vector>
#include <compileforge/core/result.hpp>
#include <compileforge/core/types.hpp>

namespace compileforge {

struct ParseFileResult {
    std::vector<IncludeDirective> includes;
    SourceMetricsData metrics;
};

class IncludeParser {
public:
    static Result<ParseFileResult> parse_file(const std::string& filepath);
    static ParseFileResult parse_content(const std::string& content, const std::string& filepath = "");

    static std::string resolve_include_path(
        const IncludeDirective& directive,
        const std::string& current_file_path,
        const std::vector<std::string>& include_search_dirs,
        const std::string& project_root
    );
};

} // namespace compileforge

#endif // COMPILEFORGE_PARSER_INCLUDE_PARSER_HPP
