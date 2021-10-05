#include "tone/core/identifier.hpp"

namespace tone::core {

    identifier_info::identifier_info(type_handle type_id, std::size_t index, bool is_global,
                                     bool is_constant)
        : _type_id(type_id)
        , _idx(index)
        , _is_global(is_global)
        , _is_constant(is_constant)
    {}

    type_handle identifier_info::type_id() const
    {
        return _type_id;
    }
    std::size_t identifier_info::index() const
    {
        return _idx;
    }
    bool identifier_info::is_global() const
    {
        return _is_global;
    }
    bool identifier_info::is_constant() const
    {
        return _is_constant;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// `identifier_lookup` class
    ////////////////////////////////////////////////////////////////////////////////////////////////

    const identifier_info* identifier_lookup::find(const std::string& name) const
    {
        if (auto it = _idents.find(name); it != _idents.end())
            return &it->second;
        return nullptr;
    }
    identifier_lookup::~identifier_lookup() = default;

    const identifier_info* identifier_lookup::insert_ident(std::string name, type_handle type_id,
                                                           std::size_t index, bool is_global,
                                                           bool is_constant)
    {
        return &_idents.emplace(std::move(name),
                                identifier_info(type_id, index, is_global, is_constant))
                        .first->second;
    }

    std::size_t identifier_lookup::idents_size() const
    {
        return _idents.size();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// `global_identifier_lookup` class
    ////////////////////////////////////////////////////////////////////////////////////////////////

    const identifier_info* global_identifier_lookup::create_identifier(std::string name,
                                                                       type_handle type_id,
                                                                       bool is_constant)
    {
        return insert_ident(name, type_id, idents_size(), true, is_constant);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// `local_identifier_lookup` class
    ////////////////////////////////////////////////////////////////////////////////////////////////

    local_identifier_lookup::local_identifier_lookup(
            std::unique_ptr<local_identifier_lookup> parent)
        : _parent(std::move(parent))
        , _next_ident_idx(_parent ? _parent->_next_ident_idx : 1)
    {}

    const identifier_info* local_identifier_lookup::find(const std::string& name) const
    {
        if (const auto ident = identifier_lookup::find(name))
            return ident;
        return _parent ? _parent->find(name) : nullptr;
    }
    const identifier_info* local_identifier_lookup::create_identifier(std::string name,
                                                                      type_handle type_id,
                                                                      bool is_constant)
    {
        return insert_ident(std::move(name), type_id, _next_ident_idx++, false, is_constant);
    }
    std::unique_ptr<local_identifier_lookup> local_identifier_lookup::detach_parent()
    {
        return std::move(_parent);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// `function_identifier_lookup` class
    ////////////////////////////////////////////////////////////////////////////////////////////////

    function_identifier_lookup::function_identifier_lookup()
        : local_identifier_lookup(nullptr)
        , _next_param_idx(-1)
    {}

    const identifier_info* function_identifier_lookup::create_param(std::string name,
                                                                    type_handle type_id)
    {
        return insert_ident(std::move(name), type_id, _next_param_idx--, false, false);
    }
} // namespace tone::core