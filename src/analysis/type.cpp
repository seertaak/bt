#include <boost/hana/all.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/zip.hpp>

#include <bullet/analysis/type.hpp>

using namespace std;

namespace rng = ranges;
namespace views = rng::views;

namespace bt { namespace analysis {
    namespace types {
        atomic<int> nominal_type_t::next_id{0};

        nominal_type_t::nominal_type_t(string fqn, type_t type): id(++nominal_type_t::next_id),
            fqn(fqn), type(type) {}

        auto operator<<(ostream& os, const nominal_type_t& n) -> ostream& {
            os << n.fqn;
            return os;
        }

        auto operator<<(ostream& os, const void_t&) -> ostream& {
            os << "void";
            return os;
        }

        auto operator<<(ostream& os, const bool_t&) -> ostream& {
            os << "bool";
            return os;
        }

        auto operator<<(ostream& os, const char_t&) -> ostream& {
            os << "char";
            return os;
        }

        auto operator<<(ostream& os, const name_and_type_t& n) -> ostream& {
            os << n.name << ":" << n.type;
            return os;
        }

        auto operator==(const name_and_type_t& lhs, const name_and_type_t& rhs) -> bool {
            return lhs.name == rhs.name && lhs.type == rhs.type;
        }

        auto operator!=(const name_and_type_t& lhs, const name_and_type_t& rhs) -> bool {
            return !(lhs == rhs);
        }

        auto operator==(const name_and_type_vector_t& lhs, const name_and_type_vector_t& rhs)
            -> bool {
            return rng::all_of(views::zip(lhs, rhs),
                               [](auto&& e) { return get<0>(e) == get<1>(e); });
        }

        auto operator!=(const name_and_type_vector_t& lhs, const name_and_type_vector_t& rhs)
            -> bool {
            return !(lhs == rhs);
        }

        auto operator<<(ostream& os, const name_and_type_vector_t& v) -> ostream& {
            auto first = true;
            for (auto&& nt : v) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << nt;
            }
            return os;
        }

        auto operator==(const function_t& lhs, const function_t& rhs) -> bool {
            return lhs.result_type == rhs.result_type &&
                   lhs.formal_parameters == rhs.formal_parameters;
        }

        auto operator!=(const function_t& lhs, const function_t& rhs) -> bool {
            return !(lhs == rhs);
        }

        auto operator<<(ostream& os, const function_t& fn) -> ostream& {
            os << "function(" << fn.result_type << ", " << fn.formal_parameters << ")";
            return os;
        }

        auto operator==(const struct_t& lhs, const struct_t& rhs) -> bool {
            return static_cast<const struct_t&>(lhs) == static_cast<const struct_t&>(rhs);
        }
        auto operator!=(const struct_t& lhs, const struct_t& rhs) -> bool { return !(lhs == rhs); }
        auto operator<<(ostream& os, const struct_t& v) -> ostream& {
            os << "struct(";
            auto first = true;
            for (auto&& nt : v) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << nt;
            }
            os << ")";
            return os;
        }

        auto operator==(const tuple_t& lhs, const tuple_t& rhs) -> bool {
            return rng::all_of(views::zip(lhs, rhs),
                               [](auto&& e) { return get<0>(e) == get<1>(e); });
        }
        auto operator!=(const tuple_t& lhs, const tuple_t& rhs) -> bool { return !(lhs == rhs); }
        auto operator<<(ostream& os, const tuple_t& v) -> ostream& {
            os << "tuple(";
            auto first = true;
            for (auto&& t : v) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << t;
            }
            os << ")";
            return os;
        }

        auto operator==(const ptr_t& lhs, const ptr_t& rhs) -> bool {
            return lhs.value_type == rhs.value_type;
        }
        auto operator!=(const ptr_t& lhs, const ptr_t& rhs) -> bool { return !(lhs == rhs); }
        auto operator<<(ostream& os, const ptr_t& p) -> ostream& {
            os << "ptr(" << p.value_type << ")";
            return os;
        }

        auto operator==(const array_t& lhs, const array_t& rhs) -> bool {
            return lhs.value_type == rhs.value_type && lhs.size.size() == rhs.size.size() &&
                   rng::all_of(views::zip(lhs.size, rhs.size),
                               [](auto&& e) { return get<0>(e) == get<1>(e); });
        }
        auto operator!=(const array_t& lhs, const array_t& rhs) -> bool { return !(lhs == rhs); }
        auto operator<<(ostream& os, const array_t& a) -> ostream& {
            os << "array(" << a.value_type << ", ";
            auto first = true;
            for (auto&& n : a.size) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << n;
            }
            os << ")";
            return os;
        }

        auto operator==(const dynarr_t& lhs, const dynarr_t& rhs) -> bool {
            return lhs.value_type == rhs.value_type;
        }
        auto operator!=(const dynarr_t& lhs, const dynarr_t& rhs) -> bool { return !(lhs == rhs); }
        auto operator<<(ostream& os, const dynarr_t& da) -> ostream& {
            os << "dynarr(" << da.value_type << ")";
            return os;
        }

        auto operator==(const slice_t& lhs, const slice_t& rhs) -> bool {
            return lhs.value_type == rhs.value_type && lhs.begin == rhs.begin &&
                   lhs.end == rhs.end && lhs.stride == rhs.stride;
        }
        auto operator!=(const slice_t& lhs, const slice_t& rhs) -> bool { return !(lhs == rhs); }
        auto operator<<(ostream& os, const slice_t& s) -> ostream& {
            os << "slice(" << s.value_type << ", " << s.begin << ", " << s.end << ", " << s.stride
               << ")";
            return os;
        }

        auto operator==(const strlit_t& lhs, const strlit_t& rhs) -> bool {
            return lhs.size == rhs.size;
        }
        auto operator!=(const strlit_t& lhs, const strlit_t& rhs) -> bool { return !(lhs == rhs); }
        auto operator<<(ostream& os, const strlit_t& s) -> ostream& {
            os << "strlit(" << s.size << ")";
            return os;
        }

        auto operator==(const variant_t& lhs, const variant_t& rhs) -> bool {
            return rng::all_of(views::zip(lhs, rhs),
                               [](auto&& e) { return get<0>(e) == get<1>(e); });
        }
        auto operator!=(const variant_t& lhs, const variant_t& rhs) -> bool {
            return !(lhs == rhs);
        }
        auto operator<<(ostream& os, const variant_t& v) -> ostream& {
            os << "variant(";
            auto first = true;
            for (auto&& t : v) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << t;
            }
            os << ")";
            return os;
        }

        auto operator==(const string_t& lhs, const string_t& rhs) -> bool { return true; }
        auto operator!=(const string_t& lhs, const string_t& rhs) -> bool { return !(lhs == rhs); }
        auto operator<<(ostream& os, const string_t& s) -> ostream& {
            os << "string";
            return os;
        }

        auto operator==(const nominal_type_t& lhs, const nominal_type_t& rhs) -> bool { 
            return lhs.id == rhs.id;
        }

        auto operator!=(const nominal_type_t& lhs, const nominal_type_t& rhs) -> bool { return !(lhs == rhs); }

    }  // namespace types

    auto operator<<(ostream& os, const type_t& t) -> ostream& {
        os << t.get();
        return os;
    }

    auto operator<<(std::ostream& os, const type_value& t) -> std::ostream& {
        visit([&](const auto& u) { os << u; }, t);
        return os;
    }

    auto is_integral(const type_t& t) -> bool {
        return visit([](const auto& u) { return types::is_int_v<decltype(u)>; }, t.get());
    }
    auto is_floating_point(const type_t& t) -> bool {
        return visit([](const auto& u) { return types::is_float_v<decltype(u)>; }, t.get());
    }
    auto is_signed(const type_t& t) -> bool {
        return visit(
            [](const auto& u) -> bool {
                using U = remove_const_t<remove_reference_t<decltype(u)>>;
                if constexpr (types::is_int_v<U>) return U::is_signed;
                if constexpr (types::is_float_v<U>) return U::is_signed;
                return false;
            },
            t.get());
    }
    auto width(const type_t& t) -> int {
        return visit(
            [](const auto& u) {
                using U = remove_const_t<remove_reference_t<decltype(u)>>;
                if constexpr (types::is_int_v<U>) {
                    return U::width;
                }
                if constexpr (types::is_float_v<U>) {
                    return U::width;
                }
                return 0;
            },
            t.get());
    }
    auto promoted_type(const type_t& t_input, const type_t& u_input) -> optional<type_t> {
        auto t = t_input;
        while (auto pt = t.template get_if<types::ptr_t>())
            t = pt->value_type;
        
        auto u = u_input;
        while (auto pu = u.template get_if<types::ptr_t>())
            u = pu->value_type;

        if (is_floating_point(t) && is_floating_point(u))
            return optional(width(t) >= width(u) ? t : u);
        else if (is_floating_point(t) && is_integral(u))
            return width(t) > width(u) ? optional(t) : nullopt;
        else if (is_floating_point(u) && is_integral(t))
            return width(u) > width(t) ? optional(u) : nullopt;
        else if (is_integral(t) && is_integral(u)) {
            if (is_signed(t) == is_signed(u))
                return optional(width(t) >= width(u) ? t : u);
            else if (is_signed(t) && !is_signed(u))
                return width(t) > width(u) ? optional(t) : nullopt;
            else if (is_signed(u) && !is_signed(t))
                return width(u) > width(t) ? optional(u) : nullopt;
            return nullopt;
        } 
        return nullopt;
    }
    auto is_assignable_to(const type_t& src, const type_t& dst) -> bool {
        auto t_src = src;
        auto t_dst = dst;
        if (!dst.is<types::ptr_t>()) return false;

        auto dst_value_ty = t_dst.as<types::ptr_t>().value_type;

        if (dst_value_ty == t_src) return true;

        return !!promoted_type(t_src, dst_value_ty);
    }
}}  // namespace bt::analysis
