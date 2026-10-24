#include <exception>

#include "check.hpp"

int main() {
    int failed = 0;
    for (const auto& t : check::registry()) {
        try {
            t.fn();
            std::cout << "[pass] " << t.name << '\n';
        } catch (const check::Failure& f) {
            ++failed;
            std::cout << "[FAIL] " << t.name << "  " << f.message << '\n';
        } catch (const std::exception& e) {
            ++failed;
            std::cout << "[FAIL] " << t.name << "  exception: " << e.what() << '\n';
        }
    }
    std::cout << check::registry().size() - failed << '/' << check::registry().size()
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
