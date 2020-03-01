#pragma once

#include <tuple>
#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast.hpp>

namespace bt { namespace analysis {

    namespace annotated {
        template <class Node, class Annotation>
        using node_value_t = std::tuple<parser::syntax::ref<Node>, Annotation>;

        template <class Node, class Annotation>
        inline auto operator<<(std::ostream& os, const node_value_t<Node, Annotation>& v) {
            os << "annotated[node=" << std::get<0>(v).get() << ", annotation=" << std::get<1>(v) << "]";
            return os;
        }

        template <typename Node, typename Annotation>
        constexpr auto annotate = [] (parser::syntax::ref<Node>&& n, Annotation&& a) -> node_value_t<Node, Annotation> {
            return {n, a};
        };

        namespace hana = boost::hana;

        template <typename T>
        struct variant_types {
            static constexpr auto type_c = variant_types<typename T::base_t>::type_c;
        };

        template <typename...T>
        struct variant_types<std::variant<T...>> {
            static constexpr auto type_c = hana::tuple_t<T...>;
        };

        template <typename T>
        constexpr auto variant_types_tuple_c = variant_types<T>::type_c;

        constexpr auto variant_tags = [] (auto tt) {
            using t = typename decltype(tt)::type;
            return variant_types_tuple_c<t>;
        };

        constexpr auto not_a_ref = hana::int_c<-1>;

        template <typename T>
        constexpr auto ref_type_c = not_a_ref;

        template <typename T>
        constexpr auto ref_type_c<parser::syntax::ref<T>> = hana::type_c<T>;

        constexpr auto is_ref = [] (auto type) {
            return !hana::equal(ref_type_c<typename decltype(type)::type>, not_a_ref);
        };

        template <typename Tree, typename Ann>
        struct tree_t;

        template <typename Tree, typename Ann>
        using node_t = parser::syntax::ref<tree_t<Tree, Ann>>;

        constexpr auto is_recursive = [] (auto tree) {
            using tree_t = typename decltype(tree)::type;
            
            return hana::any_of(tree, [] (auto t) {
                using node_t = typename decltype(t)::type;
                return is_ref(t);// && 
                    //std::is_same_v<parser::syntax::ref<tree_t>, node_t>;
            });
        };

        constexpr auto tree_type_c = [] (auto tree, auto annotation) {
            using tree_t = typename decltype(annotation)::type;
            using annotation_t = typename decltype(annotation)::type;
            return hana::transform(
                tree,
                [] (auto n) {
                    if constexpr (is_ref(n))
                        return hana::type_c<node_t<tree_t, annotation_t>>;
                    else {
                        using raw_node_t = typename decltype(n)::type;
                        return hana::type_c<node_value_t<raw_node_t, annotation_t>>;
                    }
                });
        };


        template <typename Tree, typename Ann>
        using tree_value_t = typename decltype(
            hana::unpack(
                tree_type_c(hana::type_c<Tree>, hana::type_c<Ann>),
                hana::template_<std::variant>
            )
        )::type;

        template <typename Tree, typename Ann>
        struct tree_t : tree_value_t<Tree, Ann> {
            using base_t = tree_value_t<Tree, Ann>;
            using base_t::base_t;
        };
    }

    namespace second_try {
        namespace hana = boost::hana;
        // let Ts be a list of types.
        constexpr auto to_variant_type = [](auto type_tags) {
            return hana::unpack(
                type_tags,
                hana::template_<std::variant>
            );
        };

        template <typename...Type>
        class tree_t;

        template <typename...Type>
        using node_t = parser::syntax::ref<tree_t<Type...>>;

        template <typename...Type>
        using tree_base_t = std::variant<Type..., node_t<Type...>>;

        template <typename...Type>
        class tree_t : public tree_base_t<Type...> {
        public:
            using base_t = tree_base_t<Type...>;
            using base_t::base_t;
        };

        constexpr auto to_recursive_variant_type = [](auto variant_type_tags) {
            return hana::unpack(
                variant_type_tags,
                hana::template_<tree_t>
            );
        };

        constexpr auto annotator = [](auto annotations) {
            return [=] (auto type) {
                return hana::unpack(
                    hana::prepend(annotations, type),
                    hana::template_<std::tuple>
                );
            };
        };

        constexpr auto annotated_types = [](auto types, auto annotations) {
            return hana::transform(
                types,
                annotator(annotations)
            );
        };

        /*
        const auto f = [] (const block_t& b) -> unordered_set<identifier_t> {
            ....
        };

            
        template <typename Annotation, typename...Type, typename...Fn>
        auto transformed_tree(const tree_t<Type...>& input, Fn&&...fn)
            -> typename decltype(
                    annotated_types(
                        hana::tuple_t<Type...>, 
                        hana::tuple_t<Annotation>
                    )
               )::type
        {
            return std::visit(
                [] (const Type
                input
            );
        }
        */
    }
} }

        
    /*
    struct location_t { 
        int line, column;
    };

    struct position_t {
        location_t begin, end;
    };

    template <typename Parser>
    class position_annotator_t {
        Parser& parser;
        location_t begin;
    public:
        position_annotator_t(Parser& parser):
            parser(parser),
            begin{
                parser.peek().location.line,
                parser.peek().location.first_col
            } 
        {}

        ~position_annotator_t() {
            auto t = eat();

            ...

            auto t2 = eat();
            
        }



    };
    */

