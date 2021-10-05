#include "tone/core/type.hpp"
#include "tone/core/variant_helpers.hpp"

namespace tone::core {
    type type_registry::_void_type = primitive_type::nothing;
    type type_registry::_bool_type = primitive_type::boolean;
    type type_registry::_real_type = primitive_type::real;
    type type_registry::_int_type = primitive_type::integer;
    type type_registry::_str_type = primitive_type::str;

    type_registry::type_registry() = default;

    bool type_registry::type_less::operator()(const type& lhs, const type& rhs) const
    {
        const auto idx_l = lhs.index();
        const auto idx_r = rhs.index();

        // Types are different variants
        if (idx_l != idx_r)
            return idx_l < idx_r;

        // Types are the same variant
        if (std::holds_alternative<primitive_type>(lhs))
            return std::get<primitive_type>(lhs) < std::get<primitive_type>(rhs);
        else if (std::holds_alternative<array_type>(lhs))
            return std::get<array_type>(lhs).inner_type_id <
                   std::get<array_type>(rhs).inner_type_id;
        else if (std::holds_alternative<function_type>(lhs))
        {
            const auto& ft_l = std::get<function_type>(lhs);
            const auto& ft_r = std::get<function_type>(rhs);

            if (ft_l.return_type_id != ft_r.return_type_id)
                return ft_l.return_type_id < ft_r.return_type_id;

            if (ft_l.param_type_id.size() != ft_r.param_type_id.size())
                return ft_l.param_type_id.size() < ft_r.param_type_id.size();

            for (auto i = 0; i < ft_l.param_type_id.size(); ++i)
            {
                if (ft_l.param_type_id[i].type_id != ft_r.param_type_id[i].type_id)
                    return ft_l.param_type_id[i].type_id < ft_r.param_type_id[i].type_id;
                if (ft_l.param_type_id[i].by_ref != ft_r.param_type_id[i].by_ref)
                    return ft_r.param_type_id[i].by_ref;
            }
        }
        return false;
    }

    type_handle type_registry::get_handle(const type& t)
    {
        // clang-format off
        return std::visit(overloaded{
            [](primitive_type t) {
                if (t == primitive_type::nothing)
                    return type_registry::get_void_handle();
                if (t == primitive_type::boolean)
                    return type_registry::get_bool_handle();
                if (t == primitive_type::real)
                    return type_registry::get_real_handle();
                if (t == primitive_type::integer)
                    return type_registry::get_int_handle();
                if (t == primitive_type::str)
                    return type_registry::get_str_handle();
                return type_handle(nullptr);
            },
            [this](const auto& t) { return &(*(_types.insert(t).first)); }
        },t);
        // clang-format on
    }

    type_handle type_registry::get_void_handle()
    {
        return &_void_type;
    }
    type_handle type_registry::get_bool_handle()
    {
        return &_bool_type;
    }
    type_handle type_registry::get_real_handle()
    {
        return &_real_type;
    }
    type_handle type_registry::get_int_handle()
    {
        return &_int_type;
    }
    type_handle type_registry::get_str_handle()
    {
        return &_str_type;
    }

} // namespace tone::core