#include "../../include/compileforge/core/json.hpp"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cmath>

namespace compileforge {

const JsonValue JsonValue::null_instance = JsonValue(nullptr);

JsonValue::Type JsonValue::type() const {
    if (std::holds_alternative<std::nullptr_t>(storage_)) return Type::Null;
    if (std::holds_alternative<bool>(storage_)) return Type::Boolean;
    if (std::holds_alternative<double>(storage_)) return Type::Number;
    if (std::holds_alternative<std::string>(storage_)) return Type::String;
    if (std::holds_alternative<std::shared_ptr<ArrayType>>(storage_)) return Type::Array;
    if (std::holds_alternative<std::shared_ptr<ObjectType>>(storage_)) return Type::Object;
    return Type::Null;
}

bool JsonValue::as_bool(bool default_val) const {
    if (is_bool()) return std::get<bool>(storage_);
    return default_val;
}

double JsonValue::as_double(double default_val) const {
    if (is_number()) return std::get<double>(storage_);
    return default_val;
}

int JsonValue::as_int(int default_val) const {
    if (is_number()) return static_cast<int>(std::get<double>(storage_));
    return default_val;
}

size_t JsonValue::as_size_t(size_t default_val) const {
    if (is_number()) return static_cast<size_t>(std::get<double>(storage_));
    return default_val;
}

std::string JsonValue::as_string(const std::string& default_val) const {
    if (is_string()) return std::get<std::string>(storage_);
    return default_val;
}

const JsonValue::ArrayType& JsonValue::as_array() const {
    static const ArrayType empty_arr;
    if (is_array()) {
        const auto& ptr = std::get<std::shared_ptr<ArrayType>>(storage_);
        if (ptr) return *ptr;
    }
    return empty_arr;
}

JsonValue::ArrayType& JsonValue::as_array() {
    if (!is_array()) {
        storage_ = std::make_shared<ArrayType>();
    }
    auto& ptr = std::get<std::shared_ptr<ArrayType>>(storage_);
    if (!ptr) ptr = std::make_shared<ArrayType>();
    return *ptr;
}

const JsonValue::ObjectType& JsonValue::as_object() const {
    static const ObjectType empty_obj;
    if (is_object()) {
        const auto& ptr = std::get<std::shared_ptr<ObjectType>>(storage_);
        if (ptr) return *ptr;
    }
    return empty_obj;
}

JsonValue::ObjectType& JsonValue::as_object() {
    if (!is_object()) {
        storage_ = std::make_shared<ObjectType>();
    }
    auto& ptr = std::get<std::shared_ptr<ObjectType>>(storage_);
    if (!ptr) ptr = std::make_shared<ObjectType>();
    return *ptr;
}

bool JsonValue::contains(const std::string& key) const {
    if (!is_object()) return false;
    const auto& obj = as_object();
    return obj.find(key) != obj.end();
}

const JsonValue& JsonValue::operator[](const std::string& key) const {
    if (!is_object()) return null_instance;
    const auto& obj = as_object();
    auto it = obj.find(key);
    if (it != obj.end()) return it->second;
    return null_instance;
}

JsonValue& JsonValue::operator[](const std::string& key) {
    auto& obj = as_object();
    return obj[key];
}

const JsonValue& JsonValue::operator[](size_t index) const {
    if (!is_array()) return null_instance;
    const auto& arr = as_array();
    if (index < arr.size()) return arr[index];
    return null_instance;
}

static void escape_string(const std::string& input, std::ostringstream& ss) {
    ss << '"';
    for (char c : input) {
        switch (c) {
            case '"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    ss << c;
                }
                break;
        }
    }
    ss << '"';
}

static void dump_value(const JsonValue& val, std::ostringstream& ss, int indent, int current_level) {
    std::string indent_str = (indent >= 0) ? std::string(static_cast<size_t>(indent * current_level), ' ') : "";
    std::string next_indent_str = (indent >= 0) ? std::string(static_cast<size_t>(indent * (current_level + 1)), ' ') : "";
    std::string newline = (indent >= 0) ? "\n" : "";
    std::string space = (indent >= 0) ? " " : "";

    switch (val.type()) {
        case JsonValue::Type::Null:
            ss << "null";
            break;
        case JsonValue::Type::Boolean:
            ss << (val.as_bool() ? "true" : "false");
            break;
        case JsonValue::Type::Number: {
            double d = val.as_double();
            if (std::floor(d) == d && !std::isinf(d) && !std::isnan(d) && std::abs(d) < 1e15) {
                ss << static_cast<long long>(d);
            } else {
                ss << d;
            }
            break;
        }
        case JsonValue::Type::String:
            escape_string(val.as_string(), ss);
            break;
        case JsonValue::Type::Array: {
            const auto& arr = val.as_array();
            if (arr.empty()) {
                ss << "[]";
                break;
            }
            ss << "[" << newline;
            for (size_t i = 0; i < arr.size(); ++i) {
                ss << next_indent_str;
                dump_value(arr[i], ss, indent, current_level + 1);
                if (i + 1 < arr.size()) ss << ",";
                ss << newline;
            }
            ss << indent_str << "]";
            break;
        }
        case JsonValue::Type::Object: {
            const auto& obj = val.as_object();
            if (obj.empty()) {
                ss << "{}";
                break;
            }
            ss << "{" << newline;
            size_t count = 0;
            for (const auto& [k, v] : obj) {
                ss << next_indent_str;
                escape_string(k, ss);
                ss << ":" << space;
                dump_value(v, ss, indent, current_level + 1);
                if (++count < obj.size()) ss << ",";
                ss << newline;
            }
            ss << indent_str << "}";
            break;
        }
    }
}

std::string JsonValue::serialize(int indent) const {
    std::ostringstream ss;
    dump_value(*this, ss, indent, 0);
    return ss.str();
}

// Parser implementation
class JsonParser {
public:
    explicit JsonParser(std::string_view input) : input_(input), pos_(0) {}

    Result<JsonValue> parse() {
        skip_whitespace();
        if (pos_ >= input_.size()) {
            return Error{ErrorCode::ParseError, "Empty JSON input"};
        }
        auto res = parse_value();
        if (res.is_error()) return res;
        skip_whitespace();
        if (pos_ < input_.size()) {
            return Error{ErrorCode::ParseError, "Unexpected extra characters after JSON value"};
        }
        return res;
    }

private:
    std::string_view input_;
    size_t pos_;

    void skip_whitespace() {
        while (pos_ < input_.size()) {
            char c = input_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                ++pos_;
            } else if (c == '/' && pos_ + 1 < input_.size() && input_[pos_ + 1] == '/') {
                pos_ += 2;
                while (pos_ < input_.size() && input_[pos_] != '\n' && input_[pos_] != '\r') {
                    ++pos_;
                }
            } else {
                break;
            }
        }
    }

    Result<JsonValue> parse_value() {
        skip_whitespace();
        if (pos_ >= input_.size()) {
            return Error{ErrorCode::ParseError, "Unexpected end of JSON input"};
        }

        char c = input_[pos_];
        if (c == 'n') return parse_null();
        if (c == 't' || c == 'f') return parse_bool();
        if (c == '"') return parse_string();
        if (c == '[') return parse_array();
        if (c == '{') return parse_object();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parse_number();

        return Error{ErrorCode::ParseError, std::string("Unexpected character: ") + c};
    }

    Result<JsonValue> parse_null() {
        if (input_.substr(pos_, 4) == "null") {
            pos_ += 4;
            return JsonValue(nullptr);
        }
        return Error{ErrorCode::ParseError, "Expected 'null'"};
    }

    Result<JsonValue> parse_bool() {
        if (input_.substr(pos_, 4) == "true") {
            pos_ += 4;
            return JsonValue(true);
        }
        if (input_.substr(pos_, 5) == "false") {
            pos_ += 5;
            return JsonValue(false);
        }
        return Error{ErrorCode::ParseError, "Expected boolean true or false"};
    }

    Result<JsonValue> parse_number() {
        size_t start = pos_;
        if (input_[pos_] == '-') ++pos_;
        while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) ++pos_;
        if (pos_ < input_.size() && input_[pos_] == '.') {
            ++pos_;
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) ++pos_;
        }
        if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-')) ++pos_;
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) ++pos_;
        }

        std::string num_str(input_.substr(start, pos_ - start));
        try {
            double val = std::stod(num_str);
            return JsonValue(val);
        } catch (...) {
            return Error{ErrorCode::ParseError, "Invalid number string: " + num_str};
        }
    }

    Result<JsonValue> parse_string() {
        if (input_[pos_] != '"') return Error{ErrorCode::ParseError, "Expected '\"'"};
        ++pos_;
        std::string res;
        while (pos_ < input_.size()) {
            char c = input_[pos_++];
            if (c == '"') {
                return JsonValue(res);
            }
            if (c == '\\') {
                if (pos_ >= input_.size()) return Error{ErrorCode::ParseError, "Unterminated escape sequence"};
                char esc = input_[pos_++];
                switch (esc) {
                    case '"': res += '"'; break;
                    case '\\': res += '\\'; break;
                    case '/': res += '/'; break;
                    case 'b': res += '\b'; break;
                    case 'f': res += '\f'; break;
                    case 'n': res += '\n'; break;
                    case 'r': res += '\r'; break;
                    case 't': res += '\t'; break;
                    case 'u': {
                        if (pos_ + 4 > input_.size()) return Error{ErrorCode::ParseError, "Invalid unicode escape"};
                        std::string hex_str(input_.substr(pos_, 4));
                        pos_ += 4;
                        try {
                            int code = std::stoi(hex_str, nullptr, 16);
                            if (code < 128) {
                                res += static_cast<char>(code);
                            } else {
                                res += '?';
                            }
                        } catch (...) {
                            return Error{ErrorCode::ParseError, "Invalid hex in unicode escape"};
                        }
                        break;
                    }
                    default: res += esc; break;
                }
            } else {
                res += c;
            }
        }
        return Error{ErrorCode::ParseError, "Unterminated string literal"};
    }

    Result<JsonValue> parse_array() {
        if (input_[pos_] != '[') return Error{ErrorCode::ParseError, "Expected '['"};
        ++pos_;
        skip_whitespace();

        JsonValue::ArrayType arr;
        if (pos_ < input_.size() && input_[pos_] == ']') {
            ++pos_;
            return JsonValue(arr);
        }

        while (pos_ < input_.size()) {
            auto val_res = parse_value();
            if (val_res.is_error()) return val_res;
            arr.push_back(val_res.value());
            skip_whitespace();
            if (pos_ >= input_.size()) break;
            if (input_[pos_] == ']') {
                ++pos_;
                return JsonValue(arr);
            }
            if (input_[pos_] == ',') {
                ++pos_;
                skip_whitespace();
            } else {
                return Error{ErrorCode::ParseError, "Expected ',' or ']' in array"};
            }
        }
        return Error{ErrorCode::ParseError, "Unterminated array"};
    }

    Result<JsonValue> parse_object() {
        if (input_[pos_] != '{') return Error{ErrorCode::ParseError, "Expected '{'"};
        ++pos_;
        skip_whitespace();

        JsonValue::ObjectType obj;
        if (pos_ < input_.size() && input_[pos_] == '}') {
            ++pos_;
            return JsonValue(obj);
        }

        while (pos_ < input_.size()) {
            skip_whitespace();
            if (input_[pos_] != '"') {
                return Error{ErrorCode::ParseError, "Expected string key in object"};
            }
            auto key_res = parse_string();
            if (key_res.is_error()) return key_res;
            std::string key = key_res.value().as_string();

            skip_whitespace();
            if (pos_ >= input_.size() || input_[pos_] != ':') {
                return Error{ErrorCode::ParseError, "Expected ':' after object key"};
            }
            ++pos_; // skip ':'

            auto val_res = parse_value();
            if (val_res.is_error()) return val_res;
            obj[key] = val_res.value();

            skip_whitespace();
            if (pos_ >= input_.size()) break;
            if (input_[pos_] == '}') {
                ++pos_;
                return JsonValue(obj);
            }
            if (input_[pos_] == ',') {
                ++pos_;
                skip_whitespace();
            } else {
                return Error{ErrorCode::ParseError, "Expected ',' or '}' in object"};
            }
        }
        return Error{ErrorCode::ParseError, "Unterminated object"};
    }
};

Result<JsonValue> JsonValue::parse(std::string_view json_str) {
    JsonParser parser(json_str);
    return parser.parse();
}

} // namespace compileforge
