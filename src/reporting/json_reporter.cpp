#include <compileforge/reporting/report.hpp>

namespace compileforge {

JsonValue JsonReporter::to_json(const AnalysisReport& report) {
    JsonValue::ObjectType root;

    root["schema_version"] = "1.0";

    // Health Score
    JsonValue::ObjectType health_obj;
    health_obj["score"] = report.health_score.score;
    JsonValue::ArrayType pos_arr;
    for (const auto& p : report.health_score.positive_factors) pos_arr.push_back(p);
    health_obj["positive_factors"] = pos_arr;
    JsonValue::ArrayType neg_arr;
    for (const auto& n : report.health_score.negative_factors) neg_arr.push_back(n);
    health_obj["negative_factors"] = neg_arr;
    root["health_score"] = health_obj;

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

    // Build Config Findings
    JsonValue::ArrayType config_arr;
    for (const auto& finding : report.build_config_findings) {
        JsonValue::ObjectType f_obj;
        f_obj["category"] = finding.category;
        f_obj["message"] = finding.message;
        f_obj["actionable_recommendation"] = finding.actionable_recommendation;
        config_arr.push_back(f_obj);
    }
    root["build_config_findings"] = config_arr;

    // TU Cost Profiles
    JsonValue::ArrayType tu_arr;
    for (const auto& tu : report.tu_cost_profiles) {
        JsonValue::ObjectType tu_obj;
        tu_obj["path"] = tu.relative_path;
        tu_obj["tier"] = tu_cost_tier_to_string(tu.tier);
        tu_obj["cost_score"] = tu.cost_score;
        tu_obj["transitive_headers"] = tu.transitive_headers;
        tu_arr.push_back(tu_obj);
    }
    root["tu_cost_profiles"] = tu_arr;

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
        f_obj["score_breakdown"] = file.hotspot.score_breakdown;
        files_arr.push_back(f_obj);
    }
    root["files"] = files_arr;

    // Cycles
    JsonValue::ArrayType cycles_arr;
    for (const auto& cycle : report.cycles) {
        JsonValue::ObjectType c_obj;
        c_obj["length"] = cycle.length;
        JsonValue::ArrayType path_arr;
        for (const auto& p : cycle.cycle_path) path_arr.push_back(p);
        c_obj["path"] = path_arr;
        cycles_arr.push_back(c_obj);
    }
    root["cycles"] = cycles_arr;

    // Recommendations
    JsonValue::ArrayType recs_arr;
    for (const auto& rec : report.recommendations) {
        JsonValue::ObjectType r_obj;
        r_obj["priority_id"] = rec.id;
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
