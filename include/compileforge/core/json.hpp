#ifndef COMPILEFORGE_CORE_JSON_HPP
#define COMPILEFORGE_CORE_JSON_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>
#include "result.hpp"

namespace compileforge {

class JsonValue {
public:
    enum class Type {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    };

    using ArrayType = std::vector<JsonValue>;
    using ObjectType = std::unordered_map<std::string, JsonValue>;

    JsonValue() : storage_(nullptr) {}
    JsonValue(std::nullptr_t) : storage_(nullptr) {}
    JsonValue(bool b) : storage_(b) {}
    JsonValue(double d) : storage_(d) {}
    JsonValue(int i) : storage_(static_cast<double>(i)) {}
    JsonValue(size_t s) : storage_(static_cast<double>(s)) {}
    JsonValue(std::string s) : storage_(std::move(s)) {}
    JsonValue(const char* s) : storage_(std::string(s)) {}
    JsonValue(ArrayType arr) : storage_(std::make_shared<ArrayType>(std::move(arr))) {}
    JsonValue(ObjectType obj) : storage_(std::make_shared<ObjectType>(std::move(obj))) {}

    [[nodiscard]] Type type() const;
    [[nodiscard]] bool is_null() const { return type() == Type::Null; }
    [[nodiscard]] bool is_bool() const { return type() == Type::Boolean; }
    [[nodiscard]] bool is_number() const { return type() == Type::Number; }
    [[nodiscard]] bool is_string() const { return type() == Type::String; }
    [[nodiscard]] bool is_array() const { return type() == Type::Array; }
    [[nodiscard]] bool is_object() const { return type() == Type::Object; }

    [[nodiscard]] bool as_bool(bool default_val = false) const;
    [[nodiscard]] double as_double(double default_val = 0.0) const;
    [[nodiscard]] int as_int(int default_val = 0) const;
    [[nodiscard]] size_t as_size_t(size_t default_val = 0) const;
    [[nodiscard]] std::string as_string(const std::string& default_val = "") const;
    [[nodiscard]] const ArrayType& as_array() const;
    [[nodiscard]] ArrayType& as_array();
    [[nodiscard]] const ObjectType& as_object() const;
    [[nodiscard]] ObjectType& as_object();

    [[nodiscard]] bool contains(const std::string& key) const;
    [[nodiscard]] const JsonValue& operator[](const std::string& key) const;
    JsonValue& operator[](const std::string& key);
    [[nodiscard]] const JsonValue& operator[](size_t index) const;

    [[nodiscard]] std::string serialize(int indent = -1) const;

    static Result<JsonValue> parse(std::string_view json_str);

private:
    using Storage = std::variant<
        std::nullptr_t,
        bool,
        double,
        std::string,
        std::shared_ptr<ArrayType>,
        std::shared_ptr<ObjectType>
    >;
    Storage storage_;
    static const JsonValue null_instance;
};

} // namespace compileforge

#endif // COMPILEFORGE_CORE_JSON_HPP
