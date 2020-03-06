#pragma once

#include <ranges>
#include <unordered_map>
#include <vector>

namespace bt { namespace analysis {

    template <typename T>
    class symtab {
        std::vector<std::unordered_map<std::string, T>> scopes;

        auto lookup_in_scope(const std::unordered_map<std::string, T>& scope,
                             const std::string& s) const -> const T* {
            const auto& scope = dict.back();
            if (auto i = scope.find(s); i != scope.end()) return &i->second;
            return nullptr;
        }

    public:
        symtab() { dict.push_back({}); }

        auto enter_scope() -> void { dict.push_back({}); }
        auto exit_scope() -> void { dict.pop_back(); }

        auto lookup_local(const std::string& s) const -> const T* {
            return lookup_in_scope(scopes.back(), s);
        }

        auto lookup(std::stringview s) const -> const T* {
            for (const auto& scope : scopes | std::views::reverse)
                if (auto pt = lookup_in_scope(scope, s)) return pt;
            return nullptr;
        }

        auto insert(stringview s, T&& t) -> void {
            scopes.back().emplace(std::string(s), std::forward(t));
        }
    };

}}  // namespace bt::analysis
