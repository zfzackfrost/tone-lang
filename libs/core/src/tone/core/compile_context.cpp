#include "tone/core/compile_context.hpp"

namespace tone::core {

    compile_context::compile_context()
        : _params(nullptr)
    {
    }
    type_handle compile_context::get_type_handle(const type& ty)
    {
        return _types.get_handle(ty);
    }
    const identifier_info* compile_context::find(const std::string& name) const
    {
        if (_locals)
        {
            if (const auto ident = _locals->find(name))
                return ident;
        }
        return _globals.find(name);
    }
    const identifier_info*
    compile_context::create_identifier(std::string name, type_handle type_id, bool is_constant)
    {
        if (_locals)
            return _locals->create_identifier(std::move(name), type_id, is_constant);
        return _globals.create_identifier(std::move(name), type_id, is_constant);
    }
    const identifier_info* compile_context::create_param(std::string name, type_handle type_id)
    {
        return _params->create_param(std::move(name), type_id);
    }
    void compile_context::enter_scope()
    {
        _locals = std::make_unique<local_identifier_lookup>(std::move(_locals));
    }
    bool compile_context::leave_scope()
    {
        if (!_locals)
            return false;
        if (_params == _locals.get())
            _params = nullptr;
        _locals = _locals->detach_parent();
        return true;
    }
    void compile_context::enter_function()
    {
        auto params = std::make_unique<function_identifier_lookup>();
        _params = params.get();
        _locals = std::move(params);
    }


}