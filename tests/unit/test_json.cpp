#include <tests/test_framework.hpp>
#include <compileforge/core/json.hpp>
#include <compileforge/core/utils.hpp>

using namespace compileforge;

TEST_CASE(test_json_primitives) {
    auto res_null = JsonValue::parse("null");
    ASSERT_TRUE(res_null.has_value());
    ASSERT_TRUE(res_null.value().is_null());

    auto res_true = JsonValue::parse("true");
    ASSERT_TRUE(res_true.has_value());
    ASSERT_TRUE(res_true.value().is_bool());
    ASSERT_TRUE(res_true.value().as_bool());

    auto res_false = JsonValue::parse("false");
    ASSERT_TRUE(res_false.has_value());
    ASSERT_FALSE(res_false.value().as_bool());

    auto res_num = JsonValue::parse("42.5");
    ASSERT_TRUE(res_num.has_value());
    ASSERT_TRUE(res_num.value().is_number());
    ASSERT_DOUBLE_EQ(res_num.value().as_double(), 42.5);

    auto res_str = JsonValue::parse("\"Hello \\\"CompileForge\\\"!\\n\"");
    ASSERT_TRUE(res_str.has_value());
    ASSERT_TRUE(res_str.value().is_string());
    ASSERT_EQ(res_str.value().as_string(), "Hello \"CompileForge\"!\n");
}

TEST_CASE(test_json_arrays_and_objects) {
    std::string json_data = R"({
        "name": "CompileForge",
        "version": 1.0,
        "features": ["discovery", "graph", "hotspots"],
        "active": true
    })";

    auto res = JsonValue::parse(json_data);
    ASSERT_TRUE(res.has_value());

    const auto& root = res.value();
    ASSERT_TRUE(root.is_object());
    ASSERT_EQ(root["name"].as_string(), "CompileForge");
    ASSERT_DOUBLE_EQ(root["version"].as_double(), 1.0);
    ASSERT_TRUE(root["active"].as_bool());

    ASSERT_TRUE(root["features"].is_array());
    ASSERT_EQ(root["features"].as_array().size(), 3);
    ASSERT_EQ(root["features"][0].as_string(), "discovery");
    ASSERT_EQ(root["features"][1].as_string(), "graph");
    ASSERT_EQ(root["features"][2].as_string(), "hotspots");
}

TEST_CASE(test_json_serialization) {
    JsonValue::ObjectType obj;
    obj["project"] = "CompileForge";
    obj["count"] = 100;
    
    JsonValue val(obj);
    std::string compact = val.serialize(-1);
    ASSERT_TRUE(utils::starts_with(compact, "{") || utils::ends_with(compact, "}"));

    auto parsed_back = JsonValue::parse(compact);
    ASSERT_TRUE(parsed_back.has_value());
    ASSERT_EQ(parsed_back.value()["project"].as_string(), "CompileForge");
    ASSERT_EQ(parsed_back.value()["count"].as_int(), 100);
}
