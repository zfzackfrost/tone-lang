#pragma once

#include "tone/core/type.hpp"

#include <string>
#include <unordered_map>
#include <memory>

namespace tone::core {
    class identifier_info
    {
    public:
        identifier_info(type_handle type_id, std::size_t index, bool is_global, bool is_constant);

        [[nodiscard]] type_handle type_id() const;
        [[nodiscard]] std::size_t index() const;
        [[nodiscard]] bool is_global() const;
        [[nodiscard]] bool is_constant() const;

    private:
        type_handle _type_id;
        std::size_t _idx;
        bool _is_global : 1;
        bool _is_constant : 1;
    };

    class identifier_lookup
    {
    public:
        virtual const identifier_info* find(const std::string& name) const;
        virtual const identifier_info* create_identifier(std::string name, type_handle type_id,
                                                         bool is_constant) = 0;
        virtual ~identifier_lookup();

    protected:
        const identifier_info* insert_ident(std::string name, type_handle type_id,
                                            std::size_t index, bool is_global, bool is_constant);

        std::size_t idents_size() const;

    private:
        std::unordered_map<std::string, identifier_info> _idents;
    };

    class global_identifier_lookup : public identifier_lookup
    {
    public:
        const identifier_info* create_identifier(std::string name, type_handle type_id,
                                                 bool is_constant) override;
    };

    class local_identifier_lookup : public identifier_lookup
    {
    public:
        explicit local_identifier_lookup(std::unique_ptr<local_identifier_lookup> parent);

        const identifier_info* find(const std::string& name) const override;
        const identifier_info* create_identifier(std::string name, type_handle type_id,
                                                 bool is_constant) override;
        std::unique_ptr<local_identifier_lookup> detach_parent();

    private:
        std::unique_ptr<local_identifier_lookup> _parent;
        int _next_ident_idx;
    };

    class function_identifier_lookup : public local_identifier_lookup
    {
    public:
        function_identifier_lookup();
        const identifier_info* create_param(std::string name, type_handle type_id);
    private:
        int _next_param_idx;
    };

} // namespace tone::core