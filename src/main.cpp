#include <iostream>

#include <fstream>
#include <sstream>

#include <catch2/catch.hpp>
#include <rang.hpp>
#include <range/v3/algorithm/copy.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/transform.hpp>

#include <bullet/lexer/lexer.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/parser.hpp>
#include <bullet/analysis/annotated_ast.hpp>

using namespace std;
using namespace bt;
using namespace lexer;
using namespace lexer::token;
using namespace parser;
using namespace analysis;
using namespace syntax;
using namespace rang;

constexpr auto title = R"(
     / /_  __  __/ / /__  / /_
    / __ \/ / / / / / _ \/ __/
   / /_/ / /_/ / / /  __/ /_    
  /_.___/\__,_/_/_/\___/\__/
)"sv;

int main(int argc, const char* argv[]) {
    cout << style::bold << fg::blue << title << style::reset << style::bold << fg::green
         << style::italic << "\n  fast. " << fg::blue << "expressive." << style::blink << fg::red
         << " dangerous.\n"
         << style::reset << fg::reset << endl;

    if (argc <= 1) return 0;

    auto source = [&] {
        auto input = ifstream(argv[1]);
        auto buffer = stringstream();
        buffer << input.rdbuf();
        return buffer.str();
    }();

    const lexer::output_t lex_output = source | tokenize;
    const syntax::tree_t ast = parser::details::parse(lex_output);

    cout << lex_output.tokens << endl;
    cout << endl << endl;

    auto s = std::stringstream();
    parser::pretty_print(ast, s, 0);
    cout << s.str() << endl;

    namespace hana = boost::hana;
    /*

    using vt = std::variant<int, float>;
    constexpr auto uuu = annotated::variant_tags(hana::type_c<vt>);

    static_assert(hana::equal(uuu, 
        hana::tuple_t<std::tuple<syntax::ref<int>, int>, 
        std::tuple<syntax::ref<float>, int>>));
    static_assert(hana::equal(
        bt::analysis::annotated::ref_type_c<parser::syntax::ref<int>>,
        hana::type_c<int>
    ));

    static_assert(annotated::is_ref(hana::type_c<parser::syntax::ref<int>>));
    static_assert(!annotated::is_ref(hana::type_c<int>));

    static_assert(!annotated::is_recursive(uuu));
    static_assert(annotated::is_recursive(
        annotated::variant_tags(hana::type_c<parser::syntax::node_base_t>)
    ));
    static_assert(annotated::is_recursive(
        annotated::variant_tags(hana::type_c<parser::syntax::tree_t>)
    ));
    namespace st = second_try;
    using namespace second_try;

    static_assert(
        hana::equal(
            hana::type_c<std::variant<int, float>>,
            to_variant_type(hana::tuple_t<int, float>)
        )
    );

    static_assert(
        hana::equal(
            hana::type_c<st::tree_t<int, float>>,
            to_recursive_variant_type(hana::tuple_t<int, float>)
        )
    );

    using namespace std;

    static_assert(
        hana::equal(
            hana::tuple_t<tuple<int, bool>, tuple<float, bool>>,
            annotated_types(
                hana::tuple_t<int, float>,
                hana::tuple_t<bool>
            )
        )
    );

    static_assert(
        hana::equal(
            hana::type_c<st::tree_t<tuple<int, bool>, tuple<float, bool>>>,
            to_recursive_variant_type(
                annotated_types(
                    hana::tuple_t<int, float>,
                    hana::tuple_t<bool>
                )
            )
        )
    );

    using namespace std;
    */

    namespace st = second_try;
    auto t = st::attr_tree_t<std::string, st::foo, st::bar>();
    auto u = st::attr_tree_t<float, st::foo, st::bar>();

    return 0;
}
