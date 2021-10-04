#pragma once

#include <utility>
#include <vector>

namespace tone::core {
    template <typename K, typename V>
    class lookup
    {
    public:
        using value_type = std::pair<K, V>;
        using container_type = std::vector<value_type>;

        using iterator = typename container_type::const_iterator;
        using const_iterator = iterator;

        lookup(std::initializer_list<value_type> il);

        explicit lookup(container_type c);

        const_iterator begin() const;
        const_iterator end() const;

        const_iterator find(const auto& key) const;

        [[nodiscard]] std::size_t size() const;

    private:
        container_type _container;
    };
} // namespace tone::core

#include "tone/core/lookup.inl"