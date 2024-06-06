#pragma once

#include <assert.h>
#include <deque>
#include <functional>
#include <iostream>
#include <stack>
#include <type_traits>
#include <vector>

/* Monotonic queue */

// An iterator for solving sliding window problems. 
// e.g to find the biggest value in a subsequence.
//
// O(n) for querying extrema in an interval. It's better than segment tree.

struct monotonic_increasing {};
struct monotonic_decreasing {};

template <typename Tag, typename = void>
struct is_monotonic_iterator_tag : std::false_type {};

template <typename Tag>
struct is_monotonic_iterator_tag<
    Tag, std::void_t<std::enable_if_t<
             std::conjunction_v<std::is_same<Tag, monotonic_increasing>,
                                std::is_same<Tag, monotonic_decreasing>>>>>
    : std::true_type {};

template <typename Iter, typename Comp> class monotonic_queue_iterator {
  private:
    using T = typename std::iterator_traits<Iter>::value_type;
    std::deque<T> queue;
    size_t win_size;
    Iter first;

    Comp comp;

  public:
    using difference_type = void;
    using value_type = std::deque<T>;
    using pointer = std::deque<T> *;
    using reference = std::deque<T> &;
    using const_reference = const std::deque<T> &;
    using iterator_category = std::input_iterator_tag;

    // initialize the first window.
    explicit constexpr monotonic_queue_iterator(Iter first, size_t window_size,
                                                const Comp &comp)
        : win_size(window_size)
        , first(first)
        , comp(comp) {
        auto it = first;
        auto max_it = it;

        for (size_t i = 0; i < win_size; ++i) {
            std::cout << *it << std::endl;
            if (comp(*it, *max_it)) {
                max_it = it;
            }

            if (i < win_size - 1)
                ++it;
        }

        queue.push_back(*max_it);

        if (max_it != it) {
            queue.push_back(*it);
        }
    }

    explicit constexpr monotonic_queue_iterator(Iter first, size_t win_size,
                                                monotonic_decreasing g)
        : monotonic_queue_iterator(
              first, win_size,
              [](const auto &a, const auto &b) { return a > b; }) {}

    explicit constexpr monotonic_queue_iterator(Iter first, size_t win_size,
                                                monotonic_increasing g)
        : monotonic_queue_iterator(
              first, win_size,
              [](const auto &a, const auto &b) { return a < b; }) {}

    constexpr inline friend bool
    operator==(const monotonic_queue_iterator &self,
               const monotonic_queue_iterator &other) noexcept {
        return self.win_size == other.win_size && self.first == other.first;
    }

    constexpr inline friend bool
    operator!=(const monotonic_queue_iterator &self,
               const monotonic_queue_iterator &other) noexcept {
        return !(self == other);
    }

    constexpr inline monotonic_queue_iterator<Iter, Comp> operator++() {
        auto it = ++first;
        std::advance(it, win_size - 1);

        auto qit = queue.rbegin();
        while (!queue.empty() && qit != queue.rend() && *qit < *it) {
            qit++;
        }

        queue.erase(qit.base(), queue.end());
        queue.push_back(*it);

        return *this;
    }

    constexpr inline monotonic_queue_iterator<Iter, Comp> operator++(int) {
        monotonic_queue_iterator tmp(*this);
        ++this;
        return tmp;
    }

    constexpr inline const_reference operator*() const noexcept {
        return queue;
    }
    constexpr inline const_reference operator*() noexcept { return queue; }

    constexpr pointer operator->() noexcept {
        return std::addressof(operator*());
    }
};

template <typename Iter, typename Comp, typename Tag,
          typename std::enable_if_t<is_monotonic_iterator_tag<Tag>::value>>
constexpr decltype(auto) make_monotonic_queue_iterators(Iter begin, Iter end,
                                                        size_t window_size,
                                                        Tag tag) {
    return std::make_pair(
        monotonic_queue_iterator{ begin, window_size, tag },
        monotonic_queue_iterator{ end - window_size + 1, window_size, tag });
}

template <typename Iter, typename Comp>
constexpr decltype(auto) make_monotonic_queue_iterators(Iter begin, Iter end,
                                                        size_t window_size,
                                                        const Comp &comp) {
    return std::make_pair(
        monotonic_queue_iterator{ begin, window_size, comp },
        monotonic_queue_iterator{ end - window_size + 1, window_size, comp });
}

template <typename Iter, typename Comp>
constexpr decltype(auto) make_monotonic_queue_iterators(Iter begin, Iter end,
                                                        size_t window_size) {
    return std::make_pair(
        monotonic_queue_iterator{ begin, window_size, monotonic_decreasing() },
        monotonic_queue_iterator{ end - window_size + 1, window_size,
                                  monotonic_decreasing() });
}

template <typename C, typename Comp>
constexpr decltype(auto) make_monotonic_queue_iterators(C container,
                                                        size_t window_size,
                                                        const Comp &comp) {

    return std::make_pair(
        monotonic_queue_iterator{ std::begin(container), window_size, comp },
        monotonic_queue_iterator{ std::end(container) - window_size + 1,
                                  window_size, comp });
}

template <typename Iter, typename Tag>
monotonic_queue_iterator(Iter, size_t, Tag) -> monotonic_queue_iterator<
    Iter, std::function<bool(typename std::iterator_traits<Iter>::value_type,
                             typename std::iterator_traits<Iter>::value_type)>>;

template <typename Iter>
monotonic_queue_iterator(Iter, size_t) -> monotonic_queue_iterator<
    Iter, std::function<bool(typename std::iterator_traits<Iter>::value_type,
                             typename std::iterator_traits<Iter>::value_type)>>;

/* monotonic stack */

// An iterator to find the next greater element for all elements.
// stack top is always the closest to the new element. If the new element is
// bigger then the stack top, then it's the NGE of it.

template <typename Iter, typename Comp> class monotonic_stack_iterator {
  private:
    using T = typename std::iterator_traits<Iter>::value_type;
    using stack = std::vector<T>;
    Comp comp;
    stack data;
    Iter first;

  public:
    using difference_type = void;
    using pointer = stack *;
    using reference = stack &;
    using const_reference = const stack &;
    using value_type = T;
    using iterator_category = std::input_iterator_tag;

    constexpr monotonic_stack_iterator(Iter first, const Comp &comp)
        : comp(comp)
        , data()
        , first(first) {}

    constexpr monotonic_stack_iterator(Iter first, monotonic_decreasing tag)
        : monotonic_stack_iterator(
              first, [](const auto &a, const auto &b) { return a > b; }) {}

    constexpr monotonic_stack_iterator(Iter first, monotonic_increasing tag)
        : monotonic_stack_iterator(
              first, [](const auto &a, const auto &b) { return a < b; }) {}

    constexpr friend bool operator==(const monotonic_stack_iterator &self,
                                     const monotonic_stack_iterator &other) {
        return self.first == other.first;
    }

    constexpr friend bool operator!=(const monotonic_stack_iterator &self,
                                     const monotonic_stack_iterator &other) {
        return !(self == other);
    }

    constexpr monotonic_stack_iterator<Iter, Comp> operator++() {
        auto it = first++;
        if (data.empty()) {
            data.push_back(*it);
            return *this;
        }

        while (!data.empty() && comp(*it, data.back())) {
            data.pop_back();
        }

        data.push_back(*it);
        return *this;
    }

    constexpr monotonic_stack_iterator<Iter, Comp> operator++(int) {
        monotonic_stack_iterator tmp(*this);
        ++this;
        return tmp;
    }

    constexpr const_reference operator*() { return data; }
    constexpr const_reference operator*() const { return data; }
};

template <typename Iter>
monotonic_stack_iterator(Iter) -> monotonic_stack_iterator<
    Iter, std::function<bool(typename std::iterator_traits<Iter>::value_type,
                             typename std::iterator_traits<Iter>::value_type)>>;

template <typename Iter, typename Tag>
monotonic_stack_iterator(Iter, Tag) -> monotonic_stack_iterator<
    Iter, std::function<bool(typename std::iterator_traits<Iter>::value_type,
                             typename std::iterator_traits<Iter>::value_type)>>;

template <typename Iter, typename Comp>
constexpr decltype(auto) make_monotonic_stack_iterators(Iter begin, Iter end,
                                                        const Comp &comp) {
    return std::pair(monotonic_stack_iterator{ begin, comp },
                     monotonic_stack_iterator{ end + 1, comp });
}

template <typename Iter, typename Comp, typename Tag,
          typename std::enable_if_t<is_monotonic_iterator_tag<Tag>::value>>
constexpr decltype(auto) make_monotonic_stack_iterators(Iter begin, Iter end,
                                                        Tag tag) {
    return std::pair(monotonic_stack_iterator{ begin, tag },
                     monotonic_stack_iterator{ end + 1, tag });
}
