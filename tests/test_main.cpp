#include <exception>
#include <iostream>

#include "TestSupport.hpp"

int main() {
    int failures = 0;

    for (const auto& test : traffic::tests::registry()) {
        try {
            test.function();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
        }
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << traffic::tests::registry().size() << " test(s) passed\n";
    return 0;
}
