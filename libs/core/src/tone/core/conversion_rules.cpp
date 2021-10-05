#include "tone/core/conversion_rules.hpp"

namespace tone::core {

    bool is_convertable(type_handle type_from, bool lvalue_from, type_handle type_to,
                        bool lvalue_to)
    {
        if (type_to == type_registry::get_void_handle())
            return true;
        if (lvalue_to)
            return lvalue_from && type_from == type_to;
        if (type_from == type_to)
            return true;

        // ============== Allowed implicit conversions ============== //

        // int to str
        if (type_from == type_registry::get_int_handle() && type_to == type_registry::get_str_handle())
            return true;
        // real to str
        if (type_from == type_registry::get_real_handle() && type_to == type_registry::get_str_handle())
            return true;
        // int to real
        if (type_from == type_registry::get_int_handle() && type_to == type_registry::get_real_handle())
            return true;
        // real to int
        if (type_from == type_registry::get_real_handle() && type_to == type_registry::get_int_handle())
            return true;
        // int to bool
        if (type_from == type_registry::get_int_handle() && type_to == type_registry::get_bool_handle())
            return true;
        // real to bool
        if (type_from == type_registry::get_real_handle() && type_to == type_registry::get_bool_handle())
            return true;

        return false;
    }
}