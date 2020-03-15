#include <list>

#include <boost/hana/all.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/core.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/zip.hpp>

#include <bullet/analysis/type.hpp>

using namespace std;

namespace rng = ranges;
namespace views = rng::views;

namespace bt { namespace analysis {
    namespace types {
        atomic<int> nominal_type_t::next_id{0};

        nominal_type_t::nominal_type_t(string fqn, type_t type)
            : id(++nominal_type_t::next_id), fqn(fqn), type(type) {}

        auto operator<<(ostream& os, const nominal_type_t& n) -> ostream& {
            os << n.fqn;
            return os;
        }

        auto operator<<(ostream& os, const placeholder_t&) -> ostream& {
            os << "<placeholder>";
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

        auto operator<<(ostream& os, const floatlit_t& v) -> ostream& {
            os << "floatlit";
            return os;
        }

        auto operator<<(ostream& os, const intlit_t& v) -> ostream& {
            os << "intlit";
            return os;
        }

        auto operator==(const nominal_type_t& lhs, const nominal_type_t& rhs) -> bool {
            return lhs.id == rhs.id;
        }

        auto operator!=(const nominal_type_t& lhs, const nominal_type_t& rhs) -> bool {
            return !(lhs == rhs);
        }

        auto operator==(const placeholder_t&, const placeholder_t&) -> bool { return true; }
        auto operator!=(const placeholder_t&, const placeholder_t&) -> bool { return true; }

        auto operator==(const intlit_t&, const intlit_t&) -> bool { return true; }
        auto operator==(const floatlit_t&, const floatlit_t&) -> bool { return true; }
        auto operator!=(const intlit_t&, const intlit_t&) -> bool { return false; }
        auto operator!=(const floatlit_t&, const floatlit_t&) -> bool { return false; }

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
        /*
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
        */

        auto d_u = implicit_conversion_distance(t_input, u_input);
        auto d_t = implicit_conversion_distance(u_input, t_input);

        if (d_u < 0 && d_t < 0) return nullopt;
        if (d_u >= 0) return u_input;
        if (d_t >= 0) return t_input;

        return d_u < d_t ? u_input : t_input;
    }

    auto ptr_depth(type_t t) -> int {
        auto depth = 0;

        while (auto p = t->get_if<types::ptr_t>()) {
            t = p->value_type;
            depth++;
        }

        return depth;
    }

    auto is_convertible_to(const type_t& src, const type_t& dst) -> bool {
        return implicit_conversion_distance(src, dst) >= 0;
    }

    auto is_assignable_to(const type_t& src, const type_t& dst) -> bool {
        const auto d_src = ptr_depth(src);
        const auto d_dst = ptr_depth(dst);
        if (d_dst - 1 > d_src) return false;
        return is_convertible_to(src, dst->as<types::ptr_t>().value_type);
    }

    auto deref(const type_t& t) -> type_t {
        return t->is<types::ptr_t>() ? deref(t->as<types::ptr_t>().value_type) : t;
    }

    auto decay_ptr(type_t p) -> type_t { return decay_ptr(p, ptr_depth(p)); }

    auto decay_ptr(type_t p, int n_levels) -> type_t {
        for (auto i = 0; i < n_levels; i++) p = p->get<types::ptr_t>().value_type;
        return p;
    }

    auto is_immutable(const type_t& t) -> bool {
        return visit(
            hana::overload([&](const types::ptr_t& p) { return false; },
                           [&](const types::variant_t& t) {
                               return rng::all_of(t, [](auto&& u) { return is_immutable(u); });
                           },
                           [&](const types::tuple_t& t) {
                               return rng::all_of(t, [](auto&& u) { return is_immutable(u); });
                           },
                           [&](const types::array_t& t) { return is_immutable(t.value_type); },
                           [&](const types::dynarr_t& t) { return is_immutable(t.value_type); },
                           [&](const auto& t) { return true; }),
            t.get());
    }

    auto implicit_conversion_distance(const type_t& src, const type_t& dst) -> int {
        auto& d = dst.get();

        auto candidates = std::list<type_t>();
        candidates.push_back(src);

        int length = 0;

        while (!candidates.empty()) {
            const auto& candidate_ref = candidates.front();
            const auto& candidate = candidate_ref.get();

            if (candidate == d) {
                cout << "Implicit conversion chain found from " << src << " to " << dst
                     << " of length " << length << endl;
                return length;
            }

            if (auto pnom = d.get_if<types::nominal_type_t>();
                !!pnom && (candidate == pnom->type)) {
                return length;
            }

            using namespace types;

            visit(
                hana::overload(
                    [&](const types::placeholder_t&) { candidates.push_back(dst); },
                    [&](const types::intlit_t&) {
                        candidates.push_back(U8);
                        candidates.push_back(I8);
                    },
                    [&](const types::floatlit_t&) { candidates.push_back(F32); },
                    [&](const types::i8_t&) { candidates.push_back(I16); },
                    [&](const types::i16_t&) { candidates.push_back(I32); },
                    [&](const types::i32_t&) {
                        candidates.push_back(I64);
                        candidates.push_back(F32);
                    },
                    [&](const types::i64_t&) { candidates.push_back(F64); },
                    [&](const types::u8_t&) {
                        candidates.push_back(U16);
                        candidates.push_back(I16);
                    },
                    [&](const types::u16_t&) {
                        candidates.push_back(U32);
                        candidates.push_back(I32);
                    },
                    [&](const types::u32_t&) {
                        candidates.push_back(U64);
                        candidates.push_back(I64);
                    },
                    [&](const types::u64_t&) { candidates.push_back(F64); },
                    [&](const types::ptr_t& p) { candidates.push_back(p.value_type); },
                    [&](const types::f32_t& p) { candidates.push_back(F64); },
                    [&](const types::function_t& f) {
                        if (f.formal_parameters.empty()) candidates.push_back(f.result_type);
                    },
                    [&](const types::variant_t& t) {
                        if (t.empty()) return;
                        if (t.size() == 1) candidates.push_back(t.front());

                        auto u = rng::accumulate(t | views::tail, t.front(),
                                                 [](auto uni_t, auto t) -> type_t {
                                                     if (auto p = promoted_type(uni_t, t))
                                                         return *p;
                                                     return UNKOWN;
                                                 });

                        if (u != UNKOWN) candidates.push_back(u);
                    },
                    [&](const types::tuple_t& t) {
                        auto tys = std::vector<type_t>();
                        if (t.size() >= 1) {
                            bool all_same = true;
                            const auto& first_ty = t.front();
                            for (const auto& ty : t) {
                                if (ty != first_ty) {
                                    all_same = false;
                                    break;
                                }
                            }

                            if (all_same)
                                candidates.push_back(
                                    type_t(type_value(types::array_t{first_ty, {t.size()}})));
                        }
                    },
                    [&](const types::array_t& t) {
                        candidates.push_back(type_value(types::dynarr_t{t.value_type}));
                        if (t.size.front() == 1) {
                            auto nsz = t.size;
                            nsz.erase(nsz.begin());
                            if (nsz.empty())
                                candidates.push_back(t.value_type);
                            else
                                candidates.push_back(type_value(types::array_t{t.value_type, nsz}));
                        } else if (t.size.back() == 1) {
                            auto nsz = t.size;
                            nsz.pop_back();
                            if (nsz.empty())
                                candidates.push_back(t.value_type);
                            else
                                candidates.push_back(type_value(types::array_t{t.value_type, nsz}));
                        }
                        // TODO: slice!
                    },
                    [&](const types::strlit_t& t) {
                        candidates.push_back(type_value(types::string_t{}));
                    },
                    [&](const auto& t) {
                        // candidates.push_back(type_value(types::tuple_t{candidate}));
                    }),
                candidate);

            candidates.pop_front();
            length++;
        }

        return numeric_limits<int>::min();
    }
}}  // namespace bt::analysis
