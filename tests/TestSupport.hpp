#pragma once

#include <cmath>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace traffic::tests {

using TestFunction = std::function<void()>;

struct TestCase {
    std::string name;
    TestFunction function;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct TestRegistrar {
    TestRegistrar(std::string name, TestFunction function) {
        registry().push_back({std::move(name), std::move(function)});
    }
};

inline void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

inline void require_near(double actual, double expected, double epsilon, const std::string& message) {
    if (std::fabs(actual - expected) > epsilon) {
        throw std::runtime_error(
            message + " expected=" + std::to_string(expected) + " actual=" + std::to_string(actual));
    }
}

}  // namespace traffic::tests

#define TRAFFIC_TEST(Name)                                                                           \
    static void Name();                                                                              \
    static ::traffic::tests::TestRegistrar Name##_registrar {#Name, Name};                          \
    static void Name()
