#ifndef COMPILEFORGE_VALIDATION_BUILD_OBSERVER_HPP
#define COMPILEFORGE_VALIDATION_BUILD_OBSERVER_HPP

#include <string>
#include <vector>
#include "prediction_model.hpp"
#include "../core/result.hpp"

namespace compileforge {

class BuildObserver {
public:
    static Result<BuildObservation> observe_command(const std::string& build_cmd, const std::string& working_dir = ".");
    static Result<BuildObservation> parse_build_log(const std::string& log_content_or_filepath);
    static std::vector<std::string> extract_compiled_sources_from_log(const std::string& log_text);
};

} // namespace compileforge

#endif // COMPILEFORGE_VALIDATION_BUILD_OBSERVER_HPP
