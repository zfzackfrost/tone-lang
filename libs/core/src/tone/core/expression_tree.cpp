#include "tone/core/expression_tree.hpp"
#include "tone/core/compile_context.hpp"
#include "tone/core/conversion_rules.hpp"
#include "tone/core/errors.hpp"
#include "tone/core/variant_helpers.hpp"

namespace tone::core {

    node::node(compile_context& context, node_value value, std::vector<node_ptr> children,
               std::size_t line_number, std::size_t char_index)
        : _value(std::move(value))
        , _children(std::move(children))
        , _line_number(line_number)
        , _char_index(char_index)
    {
        const auto void_handle = type_registry::get_void_handle();
        const auto real_handle = type_registry::get_real_handle();
        const auto int_handle = type_registry::get_int_handle();
        const auto bool_handle = type_registry::get_bool_handle();
        const auto str_handle = type_registry::get_str_handle();
        // clang-format off
        std::visit(overloaded {
            [&](const std::u16string& value) {
                _type_id = str_handle;
                _lvalue = false;
            },
            [&](double value) {
                _type_id = real_handle;
                _lvalue = false;
            },
            [&](std::int64_t value) {
                _type_id = int_handle;
                _lvalue = false;
            },
            [&](bool value) {
                _type_id = bool_handle;
                _lvalue = false;
            },
            [&](const identifier& value) {
                if (const auto ident = context.find(value.name))
                {
                    _type_id = ident->type_id();
                    _lvalue = !ident->is_constant();
                    return;
                }
                throw undeclared_error(value.name, _line_number, _char_index);
            },
            [&](node_operation value) {
                switch(value)
                {
                case node_operation::param:
                    _type_id = _children[0]->_type_id;
                    _lvalue = false;
                    break;
                case node_operation::pre_increment:
                case node_operation::pre_decrement:
                    _type_id = _children[0]->_type_id;
                    _lvalue = true;
                    _children[0]->check_any_conversion({real_handle, int_handle}, true);
                    break;
                case node_operation::post_increment:
                case node_operation::post_decrement:
                    _type_id = _children[0]->_type_id;
                    _lvalue = false;
                    _children[0]->check_any_conversion({real_handle, int_handle}, true);
                    break;
                case node_operation::unary_plus:
                case node_operation::unary_minus:
                    _type_id = _children[0]->_type_id;
                    _lvalue = false;
                    _children[0]->check_any_conversion({real_handle, int_handle}, false);
                    break;
                case node_operation::logical_not:
                    _type_id = bool_handle;
                    _lvalue = false;
                    _children[0]->check_conversion(bool_handle, false);
                    break;
                case node_operation::bitwise_not:
                    _type_id = int_handle;
                    _lvalue = false;
                    _children[0]->check_conversion(int_handle, false);
                    break;
                case node_operation::add:
                    _type_id = _children[0]->_type_id;
                    _lvalue = false;
                    if (_children[0]->is_numeric() && _children[1]->is_numeric())
                    {
                        _children[0]->check_any_conversion({real_handle, int_handle}, false);
                        _children[1]->check_any_conversion({real_handle, int_handle}, false);
                    }
                    else if (_children[0]->is_str() || _children[1]->is_str())
                    {
                        _children[0]->check_conversion(str_handle, false);
                        _children[1]->check_conversion(str_handle, false);
                    }
                    else
                    {
                        throw syntax_error("Invalid operands for +", _line_number, _char_index);
                    }
                    break;
                case node_operation::sub:
                case node_operation::mul:
                case node_operation::div:
                case node_operation::mod:
                    _type_id = _children[0]->_type_id;
                    _lvalue = false;
                    _children[0]->check_any_conversion({real_handle, int_handle}, false);
                    _children[1]->check_any_conversion({real_handle, int_handle}, false);
                    break;
                case node_operation::bitwise_and:
                case node_operation::bitwise_or:
                case node_operation::bitwise_xor:
                case node_operation::shift_l:
                case node_operation::shift_r:
                    _type_id = int_handle;
                    _lvalue = false;
                    _children[0]->check_conversion(int_handle, false);
                    _children[1]->check_conversion(int_handle, false);
                    break;
                case node_operation::logical_and:
                case node_operation::logical_or:
                    _type_id = bool_handle;
                    _lvalue = false;
                    _children[0]->check_conversion(bool_handle, false);
                    _children[1]->check_conversion(bool_handle, false);
                    break;
                case node_operation::equal:
                case node_operation::not_equal:
                case node_operation::less:
                case node_operation::greater:
                case node_operation::less_equal:
                case node_operation::greater_equal:
                    _type_id = bool_handle;
                    _lvalue = false;
                    _children[0]->check_any_conversion({real_handle, int_handle}, false);
                    _children[1]->check_any_conversion({real_handle, int_handle}, false);
                    break;
                case node_operation::assign:
                    _type_id = _children[0]->get_type_id();
                    _lvalue = true;
                    _children[0]->check_conversion(_type_id, true);
                    _children[1]->check_conversion(_type_id, false);
                    break;
                case node_operation::add_assign:
                case node_operation::sub_assign:
                case node_operation::mul_assign:
                case node_operation::div_assign:
                case node_operation::mod_assign:
                    _type_id = _children[0]->get_type_id();
                    _lvalue = true;
                    _children[0]->check_any_conversion({real_handle, int_handle}, true);
                    _children[1]->check_any_conversion({real_handle, int_handle}, false);
                    break;
                case node_operation::comma:
                    for (int i = 0; i < int(children.size()) - 1; ++i)
                        _children[i]->check_conversion(void_handle, false);
                    _type_id = _children.back()->get_type_id();
                    _lvalue = _children.back()->is_lvalue();
                    break;
                case node_operation::index:
                    if (const auto arr = std::get_if<array_type>(_children[0]->get_type_id()))
                    {
                        _type_id = arr->inner_type_id;
                        _lvalue = _children[0]->is_lvalue();
                    }
                    else
                    {
                        throw semantic_error(
                            dump_type_handle(_children[0]->get_type_id()) +
                                    " is not indexable",
                            _line_number,
                            _char_index
                        );
                    }
                    break;
                case node_operation::call:
                    if (const auto* fn = std::get_if<function_type>(_children[0]->get_type_id()))
                    {
                        _type_id = fn->return_type_id;
                        _lvalue = false;
                        if (fn->param_type_id.size() +1 != _children.size())
                        {
                            throw semantic_error(
                                "Incorrect number of arguments. Expected " +
                                std::to_string(fn->param_type_id.size()) +
                                ", given " + std::to_string(_children.size() - 1),
                                _line_number, _char_index);
                        }
                        for (std::size_t i = 0; i < fn->param_type_id.size(); ++i)
                        {
                            if (_children[i + 1]->is_lvalue() && !fn->param_type_id[i].by_ref)
                            {
                                throw semantic_error("Function doesn't recieve the argument by reference",
                                    _children[i + 1]->_line_number, _children[i + 1]->_char_index);
                            }
                            _children[i + 1]->check_conversion(fn->param_type_id[i].type_id, fn->param_type_id[i].by_ref);
                        }
                    }
                    else
                    {
                        throw semantic_error(dump_type_handle(_children[0]->get_type_id()) + " is not callable", _line_number, _char_index);
                    }
                    break;
                }

            }
        }, _value);
        // clang-format on
    }
    bool node::is_node_operation() const
    {
        return std::holds_alternative<node_operation>(_value);
    }
    bool node::is_identifier() const
    {
        return std::holds_alternative<identifier>(_value);
    }
    bool node::is_bool() const
    {
        return std::holds_alternative<bool>(_value);
    }
    bool node::is_real() const
    {
        return std::holds_alternative<double>(_value);
    }
    bool node::is_int() const
    {
        return std::holds_alternative<std::int64_t>(_value);
    }
    bool node::is_numeric() const
    {
        return is_real() || is_int();
    }
    bool node::is_str() const
    {
        return std::holds_alternative<std::u16string>(_value);
    }


    const node_value& node::get_value() const
    {
        return _value;
    }
    const std::vector<node_ptr>& node::get_children() const
    {
        return _children;
    }
    type_handle node::get_type_id() const
    {
        return _type_id;
    }
    bool node::is_lvalue() const
    {
        return _lvalue;
    }


    void node::check_conversion(type_handle type_id, bool lvalue)
    {
        if (!is_convertable(_type_id, _lvalue, type_id, lvalue))
        {
            throw wrong_type_error(dump_type_handle(_type_id), dump_type_handle(type_id), lvalue,
                                   _line_number, _char_index);
        }
    }
    void node::check_any_conversion(std::initializer_list<type_handle> type_ids, bool lvalue)
    {
        std::string error_type;
        const char* sep = "";
        for (const auto& type_id : type_ids)
        {
            if (is_convertable(_type_id, _lvalue, type_id, lvalue))
                return;
            else
                error_type += sep + dump_type_handle(type_id);
            sep = "||";
        }
        throw wrong_type_error(dump_type_handle(_type_id), error_type, lvalue, _line_number,
                               _char_index);
    }
} // namespace tone::core