#pragma once
// Minimal test helpers so the checks need no external framework.

#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace check {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Register {
    Register(const char* name, std::function<void()> fn) {
        registry().push_back({name, std::move(fn)});
    }
};

struct Failure {
    std::string message;
};

}  // namespace check

#define CHECK_CONCAT2(a, b) a##b
#define CHECK_CONCAT(a, b) CHECK_CONCAT2(a, b)

// TEST(name) { ... }  registers a test function.
#define TEST(name)                                                        \
    static void name();                                                   \
    static check::Register CHECK_CONCAT(reg_, name)(#name, name);         \
    static void name()

#define EXPECT(cond)                                                      \
    do {                                                                  \
        if (!(cond))                                                      \
            throw check::Failure{std::string(__FILE__) + ":" +            \
                                 std::to_string(__LINE__) + ": " #cond};  \
    } while (0)

#define EXPECT_EQ(a, b) EXPECT((a) == (b))
