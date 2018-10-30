#define CATCH_CONFIG_MAIN

#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/hana.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <boost/variant.hpp>
#include <catch.hpp>
#include <range/v3/core.hpp>

#include <bullet/util.hpp>

using namespace std;

namespace hana = boost::hana;
namespace r = ranges;

using namespace hana::literals;

using r::back;
using r::front;

TEST_CASE("Hello", "[hello]") {
    REQUIRE(1 == 1);	// yup
}

namespace token {
    struct token_tag {};

    struct oparen_t : token_tag {
        static constexpr auto name = "oparen"_s;
        static constexpr auto token = "("_s;
    } oparen;

    struct cparen_t : token_tag {
        static constexpr auto name = "cparen"_s;
        static constexpr auto token = ")"_s;
    } cparen;

    template <typename T>
    auto operator<<(ostream& os, T) -> enable_if_t<is_base_of_v<token_tag, T>, ostream&> {
        os << T::name.c_str();
        return os;
    }
}  // namespace token

struct identifier_t {
    string name;
    identifier_t(string s) : name(s) {}
};

using token_t = variant<token::oparen_t, token::cparen_t, string>;
auto operator<<(ostream& os, const token_t& t) -> ostream& {
    visit([&](auto t) { os << t; }, t);
    return os;
}

TEST_CASE("Spirit X3 works", "[parser]") {
    using namespace boost::spirit;
    using x3::alnum;
    using x3::alpha;
    using x3::attr;
    using x3::double_;
    using x3::eoi;
    using x3::eol;
    using x3::eps;
    using x3::int_;
    using x3::lexeme;
    using x3::lit;
    using x3::no_skip;
    using x3::omit;
    using x3::raw;
    using x3::ascii::char_;
    using x3::ascii::space;

    using namespace token;

    const auto input = R"(
foo(bing)
   bar 
bing
)"s;

    auto margins = vector<int>{0};
    auto b_ws = begin(input);

    const auto mark_line_begin = [&](auto& ctx) {
        _val(ctx) = {};
        b_ws = begin(_where(ctx));
    };

    const auto emit_indent_parens = [&](auto& ctx) {
        const auto e_ws = begin(_where(ctx));
        const int margin = e_ws - b_ws;
        if (margin > back(margins)) {
            _val(ctx).push_back(oparen);
            margins.push_back(margin);
        } else {
            while (margin < back(margins) && !empty(margins)) {
                _val(ctx).push_back(cparen);
                margins.pop_back();
            }
        }
    };

    auto margin = x3::rule<class margin_type, vector<token_t>>("margin") =
        no_skip[eps[mark_line_begin] >> (*lit(' '))[emit_indent_parens]];

    auto identifier = x3::rule<class identifier_type, string>("identifier") =
        lexeme[(alpha | char_('_')) >> *(alnum | char_('_'))];

    constexpr auto token = [](auto t) { return lit(decltype(t)::token.c_str()) >> attr(t); };

    const auto tokens = *(identifier | token(oparen) | token(cparen));
    const auto line = margin >> tokens;
    const auto lines = line % eol;

    auto result = vector<token_t>();
    auto i = begin(input);

    auto success = phrase_parse(i, end(input), lines, lit(' '), result);
    REQUIRE(success);
    for (auto s : result) cout << "TOKEN: " << s << endl;
    // REQUIRE(input == result);
}
