#pragma once
#include <iostream>
#include <string>

#include <immer/map.hpp>

namespace bt { namespace analysis {

    template <typename T>
    struct symtab {
        using dict_t = immer::map<std::string, T>;

        dict_t scope;

    public:
        symtab() = default;
        symtab(const symtab&) = default;
        symtab(symtab&&) noexcept = default;
        symtab& operator=(const symtab&) = default;
        symtab& operator=(symtab&&) noexcept = default;
        auto operator<=>(const symtab&) const = default;

        symtab(std::string s, T t) { insert(s, t); }
        symtab(T t) { insert(t); }

        auto lookup(const std::string& s) const -> const T* { return scope.find(s); }

        auto insert(std::string s, T t) -> void { scope = scope.set(s, t); }
        auto insert(T t) -> void { insert("", t); }

        auto get() const -> const T& { return *lookup(""); }

        auto begin() const { return scope.begin(); }

        auto begin() { return scope.begin(); }

        auto end() const { return scope.end(); }

        auto end() { return scope.end(); }

        auto insert(const symtab& s) -> void {
            for (auto i = s.scope.begin(); !(i == s.scope.end()); ++i) insert(i->first, i->second);
        }

        auto print(std::ostream& os) const -> void {
            auto first = true;

            os << "symtab[";

            for (auto i = scope.begin(); !(i == scope.end()); ++i) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << i->first << " -> " << i->second;
            }

            os << "]";
        }
    };

    template <typename T>
    auto operator<<(std::ostream& os, const symtab<T>& scope) -> std::ostream& {
        scope.print(os);
        return os;
    }

}}  // namespace bt::analysis
