#include "tone/core/compile_context.hpp"
#include "tone/core/errors.hpp"
#include "tone/core/expression_parser.hpp"
#include "tone/core/expression_tree.hpp"
#include "tone/core/tokenizer.hpp"
#include "tone/core/variant_helpers.hpp"

#include <fmt/format.h>
#include <iostream>
#include <sstream>

using namespace tone::core;

std::string dump_node(const node_ptr& node)
{
    auto fmt_node_op = [&node](node_operation value) {
        switch (value)
        {
        case node_operation::param:
            return dump_node(node->get_children()[0]);
        case node_operation::pre_increment:
            return fmt::format("(++{})", dump_node(node->get_children()[0]));
        case node_operation::pre_decrement:
            return fmt::format("(--{})", dump_node(node->get_children()[0]));
        case node_operation::post_increment:
            return fmt::format("({}++)", dump_node(node->get_children()[0]));
        case node_operation::post_decrement:
            return fmt::format("({}--)", dump_node(node->get_children()[0]));
        case node_operation::unary_plus:
            return fmt::format("(+{})", dump_node(node->get_children()[0]));
        case node_operation::unary_minus:
            return fmt::format("(-{})", dump_node(node->get_children()[0]));
        case node_operation::bitwise_not:
            return fmt::format("(~{})", dump_node(node->get_children()[0]));
        case node_operation::logical_not:
            return fmt::format("(!{})", dump_node(node->get_children()[0]));
        case node_operation::add:
            return fmt::format("({}+{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::sub:
            return fmt::format("({}-{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::mul:
            return fmt::format("({}*{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::div:
            return fmt::format("({}/{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::mod:
            return fmt::format("({}%{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::bitwise_and:
            return fmt::format("({}&{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::bitwise_or:
            return fmt::format("({}|{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::bitwise_xor:
            return fmt::format("({}^{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::shift_l:
            return fmt::format("({}<<{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::shift_r:
            return fmt::format("({}>>{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::assign:
            return fmt::format("({}={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::add_assign:
            return fmt::format("({}+={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::sub_assign:
            return fmt::format("({}-={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::mul_assign:
            return fmt::format("({}*={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::div_assign:
            return fmt::format("({}/={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::mod_assign:
            return fmt::format("({}%={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::equal:
            return fmt::format("({}=={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::not_equal:
            return fmt::format("({}!={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::less:
            return fmt::format("({}<{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::greater:
            return fmt::format("({}>{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::less_equal:
            return fmt::format("({}<={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::greater_equal:
            return fmt::format("({}>={})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::comma:
            return fmt::format("({},{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::logical_and:
            return fmt::format("({}&&{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::logical_or:
            return fmt::format("({}||{})", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::index:
            return fmt::format("({}[{}])", dump_node(node->get_children()[0]),
                               dump_node(node->get_children()[1]));
        case node_operation::call:
            std::string s = dump_node(node->get_children()[0]);
            s += "(";
            {
                const char* sep = "";
                for (std::size_t i = 1; i < node->get_children().size(); ++i)
                {
                    s += sep;
                    s += (node->get_children()[i]->is_lvalue() ? "?" : "");
                    s += dump_node(node->get_children()[i]);
                    sep = ",";
                }
            }
            s += ")";
            return s;
        }
        return std::string("");
    };
    return std::visit(overloaded{[](double value) { return fmt::format("{:.06f}", value); },
                                 [](std::int64_t value) { return fmt::format("{}", value); },
                                 [](bool value) { return fmt::format("{}", value); }, fmt_node_op,
                                 [](const identifier& value) { return value.name; },
                                 [](const auto&) { return std::string(""); }},
                      node->get_value());
}

int main()
{
    compile_context context;
    context.create_identifier("a", type_registry::get_real_handle(), false);
    context.create_identifier("b", type_registry::get_real_handle(), false);
    context.create_identifier("c", type_registry::get_real_handle(), false);
    context.create_identifier("d", type_registry::get_real_handle(), true);
    context.create_identifier("e", type_registry::get_real_handle(), true);
    context.create_identifier("f", type_registry::get_real_handle(), true);

    context.create_identifier("str1", type_registry::get_str_handle(), false);
    context.create_identifier("str2", type_registry::get_str_handle(), false);
    context.create_identifier("str3", type_registry::get_str_handle(), false);
    context.create_identifier("str4", type_registry::get_str_handle(), true);
    context.create_identifier("str5", type_registry::get_str_handle(), true);
    context.create_identifier("str6", type_registry::get_str_handle(), true);

    std::string line;
    do
    {
        fmt::print("> ");
        std::getline(std::cin, line);
        if (line.empty())
            continue;
        std::istringstream ss(line);
        character_source_t input = [&ss]() {
            return ss.get();
        };

        push_back_stream strm(input);
        try {
            token_iterator it(strm);
            node_ptr n = parse_expression_tree(context, it, type_registry::get_void_handle(), false, true, false);
            fmt::print("Parsed expression: {}", dump_node(n));
        }
        catch(const error& err)
        {
            ss.clear();
            ss.seekg(0);
            print_error(err, input);
        }
    }
    while(!line.empty());

    return 0;
}