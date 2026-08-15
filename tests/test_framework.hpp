#ifndef COMPILEFORGE_TEST_FRAMEWORK_HPP
#define COMPILEFORGE_TEST_FRAMEWORK_HPP

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <chrono>
#include <cmath>

namespace compileforge::test {

struct TestFailure {
    std::string file;
    int line{0};
    std::string message;
};

class TestCase {
public:
    TestCase(std::string name, std::function<void()> func)
        : name_(std::move(name)), func_(std::move(func)) {}

    void run() {
        failures_.clear();
        auto start = std::chrono::high_resolution_clock::now();
        try {
            func_();
        } catch (const std::exception& ex) {
            add_failure(__FILE__, __LINE__, std::string("Unhandled exception: ") + ex.what());
        } catch (...) {
            add_failure(__FILE__, __LINE__, "Unhandled non-std exception");
        }
        auto end = std::chrono::high_resolution_clock::now();
        duration_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
    }

    void add_failure(const char* file, int line, const std::string& msg) {
        failures_.push_back({file, line, msg});
    }

    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] bool passed() const { return failures_.empty(); }
    [[nodiscard]] const std::vector<TestFailure>& failures() const { return failures_; }
    [[nodiscard]] double duration_ms() const { return duration_ms_; }

private:
    std::string name_;
    std::function<void()> func_;
    std::vector<TestFailure> failures_;
    double duration_ms_{0.0};
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void register_test(const std::string& name, std::function<void()> func) {
        tests_.emplace_back(name, std::move(func));
    }

    TestCase* current_test() { return current_test_; }

    int run_all() {
        int passed = 0;
        int failed = 0;
        double total_duration = 0.0;

        std::cout << "[================] Running " << tests_.size() << " tests.\n";
        for (auto& t : tests_) {
            current_test_ = &t;
            std::cout << "[ RUN      ] " << t.name() << "\n";
            t.run();
            total_duration += t.duration_ms();
            if (t.passed()) {
                std::cout << "[       OK ] " << t.name() << " (" << t.duration_ms() << " ms)\n";
                ++passed;
            } else {
                std::cout << "[  FAILED  ] " << t.name() << " (" << t.duration_ms() << " ms)\n";
                for (const auto& f : t.failures()) {
                    std::cout << "  " << f.file << ":" << f.line << ": " << f.message << "\n";
                }
                ++failed;
            }
        }
        std::cout << "[================] " << tests_.size() << " tests executed in " << total_duration << " ms.\n";
        std::cout << "[  PASSED  ] " << passed << " tests.\n";
        if (failed > 0) {
            std::cout << "[  FAILED  ] " << failed << " tests.\n";
            return 1;
        }
        return 0;
    }

private:
    std::vector<TestCase> tests_;
    TestCase* current_test_{nullptr};
};

struct TestRegistrar {
    TestRegistrar(const std::string& name, std::function<void()> func) {
        TestRegistry::instance().register_test(name, std::move(func));
    }
};

} // namespace compileforge::test

#define TEST_CASE(name) \
    static void test_func_##name(); \
    static compileforge::test::TestRegistrar registrar_##name(#name, test_func_##name); \
    static void test_func_##name()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            if (auto* cur = compileforge::test::TestRegistry::instance().current_test()) { \
                cur->add_failure(__FILE__, __LINE__, "Assertion failed: " #cond); \
            } \
            return; \
        } \
    } while(0)

#define ASSERT_FALSE(cond) \
    do { \
        if (cond) { \
            if (auto* cur = compileforge::test::TestRegistry::instance().current_test()) { \
                cur->add_failure(__FILE__, __LINE__, "Assertion failed (expected false): " #cond); \
            } \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        auto val_a = (a); \
        decltype(val_a) val_b = static_cast<decltype(val_a)>(b); \
        if (val_a != val_b) { \
            if (auto* cur = compileforge::test::TestRegistry::instance().current_test()) { \
                cur->add_failure(__FILE__, __LINE__, "Assertion failed: " #a " == " #b " (values differ)"); \
            } \
            return; \
        } \
    } while(0)

#define ASSERT_NE(a, b) \
    do { \
        auto val_a = (a); \
        auto val_b = (b); \
        if (val_a == val_b) { \
            if (auto* cur = compileforge::test::TestRegistry::instance().current_test()) { \
                cur->add_failure(__FILE__, __LINE__, "Assertion failed: " #a " != " #b " (values are equal)"); \
            } \
            return; \
        } \
    } while(0)

#define ASSERT_DOUBLE_EQ(a, b) \
    do { \
        double val_a = (a); \
        double val_b = (b); \
        if (std::abs(val_a - val_b) > 1e-6) { \
            if (auto* cur = compileforge::test::TestRegistry::instance().current_test()) { \
                cur->add_failure(__FILE__, __LINE__, "Assertion failed: " #a " approx " #b); \
            } \
            return; \
        } \
    } while(0)

#endif // COMPILEFORGE_TEST_FRAMEWORK_HPP
