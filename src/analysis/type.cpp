#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/zip.hpp>

#include <bullet/analysis/type.hpp>

using namespace std;

namespace rng = ranges;
namespace views = rng::views;

namespace bt { namespace analysis {
    namespace types {
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

    }  // namespace types

    auto operator<<(ostream& os, const type_t& t) -> ostream& {
        os << t.get();
        return os;
    }

    auto operator<<(std::ostream& os, const type_value& t) -> std::ostream& {
        visit([&](const auto& u) { os << u; }, t);
        return os;
    }
}}  // namespace bt::analysis
