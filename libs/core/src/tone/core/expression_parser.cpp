#include "tone/core/expression_parser.hpp"
#include "tone/core/compile_context.hpp"
#include "tone/core/errors.hpp"
#include "tone/core/tokenizer.hpp"
#include "tone/core/tokens.hpp"

#include <stack>

namespace tone::core {
    namespace {
        enum class operator_precedence
        {
            postfix,
            prefix,
            multiplication,
            addition,
            shift,
            comparison,
            equality,
            bitwise_and,
            bitwise_xor,
            bitwise_or,
            logical_and,
            logical_or,
            assignment,
            comma,
        };

        enum class operator_associativity
        {
            left_to_right,
            right_to_left,
        };

        struct operator_info {
            node_operation operation;
            operator_precedence precedence;
            operator_associativity associativity;
            int n_operands;
            std::size_t line_number;
            std::size_t char_index;

            operator_info(node_operation operation, std::size_t line_number, std::size_t char_index)
                : operation(operation)
                , line_number(line_number)
                , char_index(char_index)
            {
                switch (operation)
                {
                case node_operation::param:
                case node_operation::post_increment:
                case node_operation::post_decrement:
                case node_operation::index:
                case node_operation::call:
                    precedence = operator_precedence::postfix;
                    break;
                case node_operation::pre_increment:
                case node_operation::pre_decrement:
                case node_operation::unary_plus:
                case node_operation::unary_minus:
                case node_operation::bitwise_not:
                case node_operation::logical_not:
                    precedence = operator_precedence::prefix;
                    break;
                case node_operation::mul:
                case node_operation::div:
                case node_operation::mod:
                    precedence = operator_precedence::multiplication;
                    break;
                case node_operation::add:
                case node_operation::sub:
                    precedence = operator_precedence::addition;
                    break;
                case node_operation::shift_l:
                case node_operation::shift_r:
                    precedence = operator_precedence::shift;
                    break;
                case node_operation::less:
                case node_operation::greater:
                case node_operation::less_equal:
                case node_operation::greater_equal:
                    precedence = operator_precedence::comparison;
                    break;
                case node_operation::equal:
                case node_operation::not_equal:
                    precedence = operator_precedence::equality;
                    break;
                case node_operation::bitwise_and:
                    precedence = operator_precedence::bitwise_and;
                    break;
                case node_operation::bitwise_xor:
                    precedence = operator_precedence::bitwise_xor;
                    break;
                case node_operation::bitwise_or:
                    precedence = operator_precedence::bitwise_or;
                    break;
                case node_operation::logical_and:
                    precedence = operator_precedence::logical_and;
                    break;
                case node_operation::logical_or:
                    precedence = operator_precedence::logical_or;
                    break;
                case node_operation::assign:
                case node_operation::add_assign:
                case node_operation::sub_assign:
                case node_operation::mul_assign:
                case node_operation::div_assign:
                case node_operation::mod_assign:
                    precedence = operator_precedence::assignment;
                    break;
                case node_operation::comma:
                    precedence = operator_precedence::comma;
                    break;
                }

                switch (precedence)
                {
                case operator_precedence::prefix:
                case operator_precedence::assignment:
                    associativity = operator_associativity::right_to_left;
                    break;
                default:
                    associativity = operator_associativity::left_to_right;
                    break;
                }

                switch (operation)
                {
                case node_operation::post_increment:
                case node_operation::post_decrement:
                case node_operation::pre_increment:
                case node_operation::pre_decrement:
                case node_operation::unary_plus:
                case node_operation::unary_minus:
                case node_operation::bitwise_not:
                case node_operation::logical_not:
                case node_operation::call: // One or more
                    n_operands = 1;
                    break;
                default:
                    n_operands = 2;
                    break;
                }
            }
        };

        operator_info get_operator_info(reserved_token tok, bool prefix, std::size_t line_number,
                                        std::size_t char_index)
        {
            switch (tok)
            {
            case reserved_token::inc:
                return {prefix ? node_operation::pre_increment : node_operation::post_increment,
                        line_number, char_index};
            case reserved_token::dec:
                return {prefix ? node_operation::pre_decrement : node_operation::post_decrement,
                        line_number, char_index};
            case reserved_token::add:
                return {prefix ? node_operation::unary_plus : node_operation::add, line_number,
                        char_index};
            case reserved_token::sub:
                return {prefix ? node_operation::unary_minus : node_operation::sub, line_number,
                        char_index};
            case reserved_token::mul:
                return {node_operation::mul, line_number, char_index};
            case reserved_token::div:
                return {node_operation::div, line_number, char_index};
            case reserved_token::mod:
                return {node_operation::mod, line_number, char_index};
            case reserved_token::bitwise_not:
                return {node_operation::bitwise_not, line_number, char_index};
            case reserved_token::bitwise_and:
                return {node_operation::bitwise_and, line_number, char_index};
            case reserved_token::bitwise_or:
                return {node_operation::bitwise_or, line_number, char_index};
            case reserved_token::bitwise_xor:
                return {node_operation::bitwise_xor, line_number, char_index};
            case reserved_token::shift_l:
                return {node_operation::shift_l, line_number, char_index};
            case reserved_token::shift_r:
                return {node_operation::shift_r, line_number, char_index};
            case reserved_token::assign:
                return {node_operation::assign, line_number, char_index};
            case reserved_token::add_assign:
                return {node_operation::add_assign, line_number, char_index};
            case reserved_token::sub_assign:
                return {node_operation::sub_assign, line_number, char_index};
            case reserved_token::mul_assign:
                return {node_operation::mul_assign, line_number, char_index};
            case reserved_token::div_assign:
                return {node_operation::div_assign, line_number, char_index};
            case reserved_token::mod_assign:
                return {node_operation::mod_assign, line_number, char_index};
            case reserved_token::logical_not:
                return {node_operation::logical_not, line_number, char_index};
            case reserved_token::logical_and:
                return {node_operation::logical_and, line_number, char_index};
            case reserved_token::logical_or:
                return {node_operation::logical_or, line_number, char_index};
            case reserved_token::equal:
                return {node_operation::equal, line_number, char_index};
            case reserved_token::not_equal:
                return {node_operation::not_equal, line_number, char_index};
            case reserved_token::less:
                return {node_operation::less, line_number, char_index};
            case reserved_token::greater:
                return {node_operation::greater, line_number, char_index};
            case reserved_token::less_equal:
                return {node_operation::less_equal, line_number, char_index};
            case reserved_token::greater_equal:
                return {node_operation::greater_equal, line_number, char_index};
            case reserved_token::comma:
                return {node_operation::comma, line_number, char_index};
            case reserved_token::open_paren:
                return {node_operation::call, line_number, char_index};
            case reserved_token::open_square:
                return {node_operation::index, line_number, char_index};
            default:
                throw unexpected_syntax_error(dump_reserved_token(tok), line_number, char_index);
            }
        }

        bool is_end_of_expression(const token& t, bool allow_comma)
        {
            if (t.is_eof())
                return true;

            if (t.is_reserved_token())
            {
                switch (t.get_reserved_token())
                {
                case reserved_token::semicolon:
                case reserved_token::close_paren:
                case reserved_token::close_square:
                case reserved_token::colon:
                    return true;
                case reserved_token::comma:
                    return !allow_comma;
                default:
                    return false;
                }
            }
            return false;
        }

        bool is_evaluated_before(const operator_info& l, const operator_info& r)
        {
            return l.associativity == operator_associativity::left_to_right
                           ? l.precedence <= r.precedence
                           : l.precedence < r.precedence;
        }

        void pop_one_operator(std::stack<operator_info>& operator_stack,
                              std::stack<node_ptr>& operand_stack, compile_context& context,
                              std::size_t line_number, std::size_t char_index)
        {
            if (operand_stack.size() < operator_stack.top().n_operands)
                throw compiler_error("Failed to parse expression", line_number, char_index);

            std::vector<node_ptr> operands;
            operands.resize(operator_stack.top().n_operands);

            if (operator_stack.top().precedence != operator_precedence::prefix)
            {
                operator_stack.top().line_number = operand_stack.top()->line_number();
                operator_stack.top().char_index = operand_stack.top()->char_index();
            }

            for (int i = operator_stack.top().n_operands - 1; i >= 0; --i)
            {
                operands[i] = std::move(operand_stack.top());
                operand_stack.pop();
            }

            operand_stack.push(std::make_unique<node>(
                    context, operator_stack.top().operation, std::move(operands),
                    operator_stack.top().line_number, operator_stack.top().char_index));
            operator_stack.pop();
        }

        node_ptr parse_expression_tree_impl(compile_context& context, token_iterator& it,
                                            bool allow_comma, bool allow_empty)
        {
            std::stack<node_ptr> operand_stack;
            std::stack<operator_info> operator_stack;

            bool expected_operand = true;

            for (; !is_end_of_expression(*it, allow_comma); ++it)
            {
                if (it->is_reserved_token())
                {
                    operator_info oi =
                            get_operator_info(it->get_reserved_token(), expected_operand,
                                              it->get_line_number(), it->get_char_index());

                    if (oi.operation == node_operation::call && expected_operand)
                    {
                        ++it;
                        operand_stack.push(parse_expression_tree_impl(context, it, true, false));
                        if (it->value_equals(reserved_token::close_paren))
                        {
                            expected_operand = false;
                            continue;
                        }
                        else
                        {
                            throw syntax_error("Expected closing ')'", it->get_line_number(),
                                               it->get_char_index());
                        }
                    }

                    if ((oi.precedence == operator_precedence::prefix) != expected_operand)
                    {
                        throw unexpected_syntax_error(it->dump(), it->get_line_number(),
                                                      it->get_char_index());
                    }

                    if (!operator_stack.empty() && is_evaluated_before(operator_stack.top(), oi))
                    {
                        pop_one_operator(operator_stack, operand_stack, context,
                                         it->get_line_number(), it->get_char_index());
                    }

                    switch (oi.operation)
                    {
                    case node_operation::call:
                        ++it;
                        if (!it->value_equals(reserved_token::close_paren))
                        {
                            while (true)
                            {
                                bool remove_lvalue = !it->value_equals(reserved_token::bitwise_and);
                                if (!remove_lvalue)
                                {
                                    ++it;
                                }

                                node_ptr argument =
                                        parse_expression_tree_impl(context, it, false, false);
                                if (remove_lvalue)
                                {
                                    std::size_t line_number = argument->line_number();
                                    std::size_t char_index = argument->char_index();
                                    std::vector<node_ptr> argument_vector;
                                    argument_vector.push_back(std::move(argument));
                                    argument = std::make_unique<node>(
                                            context, node_operation::param,
                                            std::move(argument_vector), line_number, char_index);
                                }
                                else if (!argument->is_lvalue())
                                {
                                    throw wrong_type_error(
                                            dump_type_handle(argument->get_type_id()),
                                            dump_type_handle(argument->get_type_id()), true,
                                            argument->line_number(), argument->char_index());
                                }

                                operand_stack.push(std::move(argument));
                                ++oi.n_operands;

                                if (it->value_equals(reserved_token::close_paren))
                                    break;
                                else if (it->value_equals(reserved_token::comma))
                                    ++it;
                                else
                                    throw syntax_error("Expected ',' or closing ')'",
                                                       it->get_line_number(), it->get_char_index());
                            }
                        }
                        break;
                    case node_operation::index:
                        ++it;
                        operand_stack.push(parse_expression_tree_impl(context, it, true, false));
                        if (!it->value_equals(reserved_token::close_square))
                        {
                            throw syntax_error("Expected closing ']'", it->get_line_number(),
                                               it->get_char_index());
                        }
                        break;
                    default:
                        break;
                    }

                    operator_stack.push(oi);
                    expected_operand = (oi.precedence != operator_precedence::postfix);
                }
                else
                {
                    if (!expected_operand)
                        throw unexpected_syntax_error(it->dump(), it->get_line_number(),
                                                      it->get_char_index());
                    if (it->is_bool())
                    {
                        operand_stack.push(std::make_unique<node>(
                                context, it->get_bool(), std::vector<node_ptr>(),
                                it->get_line_number(), it->get_char_index()));
                    }
                    else if (it->is_real())
                    {
                        operand_stack.push(std::make_unique<node>(
                                context, it->get_real(), std::vector<node_ptr>(),
                                it->get_line_number(), it->get_char_index()));
                    }
                    else if (it->is_int())
                    {
                        operand_stack.push(std::make_unique<node>(
                                context, it->get_int(), std::vector<node_ptr>(),
                                it->get_line_number(), it->get_char_index()));
                    }
                    else if (it->is_str())
                    {
                        operand_stack.push(std::make_unique<node>(
                                context, it->get_str(), std::vector<node_ptr>(),
                                it->get_line_number(), it->get_char_index()));
                    }
                    else
                    {
                        operand_stack.push(std::make_unique<node>(
                                context, it->get_identifier_ref(), std::vector<node_ptr>(),
                                it->get_line_number(), it->get_char_index()));
                    }
                    expected_operand = false;
                }
            }

            if (expected_operand)
            {
                if (allow_empty && operand_stack.empty() && operator_stack.empty())
                {
                    return nullptr;
                }
                else
                {
                    throw syntax_error("Operand expected", it->get_line_number(),
                                       it->get_char_index());
                }
            }

            while (!operator_stack.empty())
            {
                pop_one_operator(operator_stack, operand_stack, context, it->get_line_number(),
                                 it->get_char_index());
            }

            if (operand_stack.size() != 1 || !operator_stack.empty())
            {
                throw compiler_error("Failed to parse expression", it->get_line_number(),
                                     it->get_char_index());
            }

            return std::move(operand_stack.top());
        }
    } // namespace
    node_ptr parse_expression_tree(compile_context& context, token_iterator& it, type_handle type_id,
                                   bool lvalue, bool allow_comma, bool allow_empty)
    {
        node_ptr n = parse_expression_tree_impl(context, it, allow_comma, allow_empty);
        n->check_conversion(type_id, lvalue);
        return n;
    }
} // namespace tone::core