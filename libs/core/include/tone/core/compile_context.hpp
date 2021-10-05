#pragma once

#include "tone/core/identifier.hpp"
#include "tone/core/type.hpp"

namespace tone::core {
    class compile_context
    {
    public:
        compile_context();

        type_handle get_type_handle(const type& ty);
        const identifier_info* find(const std::string& name) const;
        const identifier_info* create_identifier(std::string name, type_handle type_id, bool is_constant);
        const identifier_info* create_param(std::string name, type_handle type_id);

        void enter_scope();
        bool leave_scope();
        void enter_function();
    private:
        global_identifier_lookup _globals;
        function_identifier_lookup* _params;
        std::unique_ptr<local_identifier_lookup> _locals;
        type_registry _types;
    };
}