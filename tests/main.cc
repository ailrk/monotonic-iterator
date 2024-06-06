#include "../monotonic_iterator.hh"
#include <iostream>
#include <vector>

void monotonic_queue_test() {
    std::vector<int> vec{ 1, 3, 6, 2, 5, 1, 7, 5, 3, 9, 12 };

    auto [mit, mend] = make_monotonic_queue_iterators(vec.begin(), vec.end(), 4,
                                                      monotonic_decreasing());

    for (auto it = mit; it != mend; ++it) {
        auto q = *it;
        for (auto &v : q) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }
}

void monotonic_stack_test() {
    std::vector<int> vec{ 50, 30, 10, 5, 3, 1, 20 };

    auto [mit, mend] = make_monotonic_stack_iterators(vec.begin(), vec.end(),
                                                      monotonic_decreasing());

    for (auto it = mit; it != mend; ++it) {
        auto s = *it;
        for (auto &v : s) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }
}

int main(void) {
    std::cout << "==== queue ====" << std::endl;
    monotonic_queue_test();

    std::cout << "==== stack ====" << std::endl;
    monotonic_stack_test();
    return 0;
}
