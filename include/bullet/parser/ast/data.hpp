#pragma once

#include <iostream>

#include <bullet/parser/ast_fwd.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct data_t : std::vector<attr_node_t<Attr>> {
                using base_t = std::vector<attr_node_t<Attr>>;
                using base_t::base_t;
                Attr attribute;
            };

            template <typename Attr>
            auto operator==(const data_t<Attr>& l, const data_t<Attr>& r) -> bool {
                if (l.size() != r.size()) return false;
                for (auto i = 0; i < l.size(); i++)
                    if (l[i] != r[i]) return false;
                return true;
            }
            template <typename Attr>
            auto operator!=(const data_t<Attr>& l, const data_t<Attr>& r) -> bool {
                return !(l == r);
            }

            template <typename Attr>
            auto operator<<(std::ostream& os, const data_t<Attr>& data) -> std::ostream& {
                auto first = true;
                os << "data[[";
                for (const auto& pt : data) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << pt;
                }
                os << "], " << data.attribute << "]";
                return os;
            }
        }}} 
