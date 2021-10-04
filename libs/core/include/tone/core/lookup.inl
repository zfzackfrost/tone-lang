#pragma once

#include <algorithm>
#include <ranges>

namespace tone::core {
    template<typename K, typename V>
    lookup<K, V>::lookup(std::initializer_list<value_type> il)
        : _container(il)
    {
        std::ranges::sort(_container);
    }

    template<typename K, typename V>
    lookup<K, V>::lookup(lookup::container_type c)
        : _container(std::move(c))
    {
        std::ranges::sort(_container);
    }
    template<typename K, typename V>
    typename lookup<K, V>::const_iterator lookup<K, V>::begin() const
    {
        return _container.begin();
    }

    template<typename K, typename V>
    typename lookup<K, V>::const_iterator lookup<K, V>::end() const
    {
        return _container.end();
    }

    template<typename K, typename V>
    typename lookup<K, V>::const_iterator lookup<K, V>::find(const auto& key) const
    {
        using key_param_t = const decltype(key);
        auto it = std::lower_bound(begin(), end(), key,
                                   [](const value_type& p, key_param_t k) { return p.first < k; });
        return it != end() && it->first == key ? it : end();
    }
    template<typename K, typename V>
    std::size_t lookup<K, V>::size() const
    {
        return _container.size();
    }
}// namespace tone::core