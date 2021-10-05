#pragma once

#include <variant>
#include <vector>
#include <set>
#include <string>

namespace tone::core {
    enum class primitive_type
    {
        nothing,
        real,
        integer,
        boolean,
        str,
    };

    struct array_type;
    struct function_type;

    using type = std::variant<primitive_type, array_type, function_type>;
    using type_handle = const type*;

    struct array_type
    {
        type_handle inner_type_id;
    };

    struct function_type
    {
        struct param
        {
            type_handle type_id;
            bool by_ref;
        };

        type_handle  return_type_id;
        std::vector<param> param_type_id;
    };


    class type_registry
    {
    public:
        type_registry();

        type_handle get_handle(const type& t);

        static type_handle get_void_handle();
        static type_handle get_bool_handle();
        static type_handle get_real_handle();
        static type_handle get_int_handle();
        static type_handle get_str_handle();
    private:
        struct type_less
        {
            bool operator()(const type& lhs, const type& rhs) const;
        };
        std::set<type, type_less> _types;

        static type _void_type;
        static type _bool_type;
        static type _real_type;
        static type _int_type;
        static type _str_type;
    };


    std::string dump_type_handle(type_handle ty);
}