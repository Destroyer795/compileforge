#include <compileforge/reporting/report.hpp>

namespace compileforge {

JsonValue JsonReporter::to_json(const AnalysisReport& report) {
    JsonValue::ObjectType root;

    // Summary
    JsonValue::ObjectType sum_obj;
    sum_obj["total_files"] = report.summary.total_files;
    sum_obj["headers"] = report.summary.total_headers;
    sum_obj["translation_units"] = report.summary.total_translation_units;
    sum_obj["loc"] = report.summary.total_loc;
    sum_obj["sloc"] = report.summary.total_sloc;
    sum_obj["estimated_build_seconds"] = report.summary.total_estimated_build_seconds;
    sum_obj["circular_dependencies"] = report.summary.circular_dependency_count;
    sum_obj["high_fanout_headers"] = report.summary.high_fanout_header_count;
    sum_obj["high_churn_complexity_files"] = report.summary.high_churn_complexity_count;
    root["summary"] = sum_obj;

    // Files
    JsonValue::ArrayType files_arr;
    for (const auto& file : report.files) {
        JsonValue::ObjectType f_obj;
        f_obj["path"] = file.relative_path;
        f_obj["kind"] = (file.kind == FileKind::Header) ? "Header" : "TranslationUnit";
        f_obj["loc"] = file.metrics.total_lines;
        f_obj["sloc"] = file.metrics.sloc;
        f_obj["cyclomatic_complexity"] = file.metrics.cyclomatic_complexity;
        f_obj["fan_in_transitive"] = file.fan_stats.fan_in_transitive;
        f_obj["fan_out_transitive"] = file.fan_stats.fan_out_transitive;
        f_obj["hotspot_score"] = file.hotspot.total_score;
        f_obj["estimated_compile_seconds"] = file.build_time.compilation_seconds;
        f_obj["commits"] = file.git_data.commit_count;
        files_arr.push_back(f_obj);
    }
    root["files"] = files_arr;

    // Cycles
    JsonValue::ArrayType cycles_arr;
    for (const auto& cycle : report.cycles) {
        JsonValue::ObjectType c_obj;
        c_obj["length"] = cycle.length;
        JsonValue::ArrayType path_arr;
        for (const auto& p : cycle.cycle_path) {
            path_arr.push_back(p);
        }
        c_obj["path"] = path_arr;
        cycles_arr.push_back(c_obj);
    }
    root["cycles"] = cycles_arr;

    // Recommendations
    JsonValue::ArrayType recs_arr;
    for (const auto& rec : report.recommendations) {
        JsonValue::ObjectType r_obj;
        r_obj["id"] = rec.id;
        r_obj["severity"] = severity_to_string(rec.severity);
        r_obj["target_file"] = rec.target_file;
        r_obj["title"] = rec.title;
        r_obj["description"] = rec.description;
        r_obj["actionable_step"] = rec.actionable_step;
        recs_arr.push_back(r_obj);
    }
    root["recommendations"] = recs_arr;

    return root;
}

std::string JsonReporter::render(const AnalysisReport& report, int indent) {
    return to_json(report).serialize(indent);
}

} // namespace compileforge
