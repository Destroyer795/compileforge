#include "test_framework.hpp"

int main() {
    return compileforge::test::TestRegistry::instance().run_all();
}
