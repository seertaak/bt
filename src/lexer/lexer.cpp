#include <bullet/lexer/lexer.hpp>

#include <cassert>
#include <cctype>
#include <cmath>
#include <exception>
#include <sstream>
#include <string>

#include <boost/hana.hpp>

namespace bt { namespace lexer {
    using namespace std;

    namespace hana = boost::hana;
    using namespace hana::literals;

    using iterator = string_view::iterator;
    using const_iterator = string_view::const_iterator;

    namespace {
        class tokenizer_t {
            const string_view input;
            const uint32_t input_length;

            source_token_list_t tokens;
            std::vector<uint32_t> start_of_line;
            vector<pair<int16_t, bool>> margins{{0, true}};

        public:
            tokenizer_t(string_view input) : input(input), input_length(size(input)) {}

            auto process() -> output_t {
                using namespace token;

                uint32_t pos = 0;

                while (!eoi(pos)) {
                    const auto start_pos = pos;
                    start_of_line.push_back(pos);

                    if (eat_empty_line(pos)) continue;

                    eat_margin(pos);
                    eat_multiline_sep(pos);

                    for (;;) {
                        eat_spaces(pos);
                        if (!eat_token(pos)) break;
                    }

                    if (eoi(pos)) break;

                    eat_eol(pos);

                    if (pos == start_pos) {
                        const auto s = remaining_input(pos);
                        auto q = s.find('\n');
                        while (std::isspace(s[q])) --q;
                        ++q;

                        auto msg = stringstream();
                        msg << "Unable to tokenize: \"" << s.substr(0, q) << "\"" << endl;
                        msg << "Managed to process: " << tokens << ". ";
                        msg << "Input follows:\n--\n" << input << "[EOI]\n--\n";
                        if (!start_of_line.empty()) {
                            msg << "Start of lines: [";
                            auto first = true;
                            for (auto p : start_of_line) {
                                if (first)
                                    first = false;
                                else
                                    msg << ", ";
                                msg << p;
                            }
                            msg << "]";
                        }

                        throw std::runtime_error(msg.str());
                    }
                }

                pop_dedents(pos);

                return output_t{tokens, start_of_line};
            }

        private:
            auto remaining_input(uint32_t pos) const -> string_view {
                return input.substr(pos, input_length - pos);
            }

            auto eat_spaces(uint32_t& pos) -> void {
                auto comment = false;
                auto p = pos;
                for (; p < input_length; ++p) {
                    const auto c = input[p];
                    if (comment) {
                        if (c == '\n') break;
                        const auto c_next = p + 1 < input_length ? input[p + 1] : 0;
                        if (c == '\r' && c_next == '\n') break;
                    } else {
                        if (c == '-') {
                            const auto c_next = p + 1 < input_length ? input[p + 1] : 0;
                            if (c_next == '-') {
                                comment = true;
                                ++p;
                            } else {
                                break;
                            }
                        } else if (c == '\t') {
                            throw std::runtime_error("Tabs are not fucking allowed.");
                        } else if (c != ' ') {
                            break;
                        }
                    }
                }
                pos = p;
            }

            auto eat_token(uint32_t& pos) -> bool {
                if (eoi(pos)) return false;
                return eat_numeric_literal(pos) || eat_basic_token(pos) ||
                       eat_string_literal(pos) || eat_identifier(pos);
            }

            template <int base, typename Integral>
            auto eat_digits(uint32_t& pos, Integral& result) -> bool {
                result = 0;
                int n = 0;
                for (; pos < input_length; n++, pos++) {
                    const auto c = input[pos];
                    // cout << "EAT_DIGITS CONSIDERING : \"" << c << "\"" << endl;

                    int v = -1;

                    if constexpr (base == 16) {
                        if (isdigit(c)) {
                            v = c - '0';
                        } else if (c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' ||
                                   c == 'f') {
                            v = c - 'a' + 10;
                        } else if (c == 'a' || c == 'A' || c == 'b' || c == 'B' || c == 'c' ||
                                   c == 'C' || c == 'd' || c == 'D' || c == 'e' || c == 'E' ||
                                   c == 'f' || c == 'F') {
                            v = c - 'A' + 10;
                        }
                    } else if constexpr (base == 10) {
                        if (isdigit(c)) v = c - '0';
                    } else if constexpr (base == 8) {
                        if ('0' < c && c < '8') v = c - '0';
                    } else if constexpr (base == 2) {
                        if (c == '0' || c == '1') v = c - '0';
                    }

                    if (v >= 0) {
                        result = result * base + v;
                    } else if (n > 0 && c == '_') {
                        continue;
                    } else {
                        // cout << "BREAKING FROM EAT_DIGITS ON : \"" << c << "\"" << endl;
                        break;
                    }
                }
                return n > 0;
            }

            auto eat_numeric_literal(uint32_t& pos) -> bool {
                using namespace literal::numeric;
                const auto start_col = pos - start_of_line.back();
                const auto line = start_of_line.size();

                uint32_t p = pos;

                auto s = remaining_input(p);

                // cout << "EAT NUMERIC TOKEN " << s << endl;

                unsigned long long intval;

                int base;
                bool match = false;

                if (s.size() > 2 && s.substr(0, 2) == "0x"sv) {
                    p += 2;
                    match = eat_digits<16, unsigned long long>(p, intval);
                    base = 16;
                } else if (s.size() > 2 && s.substr(0, 2) == "0b"sv) {
                    p += 1;
                    match = eat_digits<2, unsigned long long>(p, intval);
                    base = 2;
                } else if (s.size() > 1 && s.front() == '0') {
                    p += 1;
                    base = 8;
                    match = eat_digits<8, unsigned long long>(p, intval);
                    if (!match) {
                        match = true;
                        intval = 0;
                        base = 10;
                    }
                } else {
                    match = eat_digits<10, unsigned long long>(p, intval);
                    base = 10;
                }

                if (!match) {
                    // cout << "RET FROM EAT_NUMERIC_LIT because no match for \"" << s << "\""
                    // << endl;
                    return false;
                } else {
                    // cout << "MATCH IN EAT_NUMERIC_LIT for \"" << remaining_input(p) << "\",
                    // value: " << intval << endl;
                }

                char c;

                long double fpval = intval;
                int width = 64;
                int exponent = 0;
                uint32_t q;
                int n_digits = 0;
                char signedness = 0;

                if (p == input_length) goto emit_integral_token;

                c = input[p];

                if (c == 'e' || c == 'E') {
                parse_exponent:

                    // floating point.
                    int exp_sign = 1;
                    c = input[++p];

                    if (c == '-') {
                        exp_sign = -1;
                        ++p;
                    } else if (c == '+') {
                        ++p;
                    }

                    if (eat_digits<10, int>(p, exponent)) {
                        fpval *= pow(static_cast<long double>(10), exp_sign * exponent);
                    } else {
                        throw std::runtime_error("Invalid floating point.");
                    }

                    if (p < input_length && input[p] == 'f') goto parse_fp_width;

                    goto emit_fp_token;
                } else if (c == '.' && isdigit(input[p + 1])) {
                    // floating point.
                    ++p;

                    unsigned long long int b;
                    q = p;

                    switch (base) {
                    case 10: match = eat_digits<10, unsigned long long>(p, b); break;
                    case 16: match = eat_digits<16, unsigned long long>(p, b); break;
                    case 8: match = eat_digits<8, unsigned long long>(p, b); break;
                    case 2: match = eat_digits<8, unsigned long long>(p, b); break;
                    default: assert(false);
                    }

                    n_digits = p - q;

                    fpval += static_cast<long double>(b) * pow(base, -n_digits);

                    if (!match) goto emit_fp_token;

                    c = input[p];

                    if (c == 'e' || c == 'E') goto parse_exponent;

                    if (c == 'f') {
                    parse_fp_width:
                        s = remaining_input(++p);
                        if (s.size() < 2) throw std::runtime_error("Fucking bad floating");
                        s = s.substr(0, 2);
                        if (s == "32"sv) {
                            width = 32;
                            p += 2;
                        } else if (s == "64"sv) {
                            p += 2;
                        } else {
                            throw std::runtime_error("Invalid integral width speecifier.");
                        }
                    }

                emit_fp_token:
                    pos = p;
                    const auto last_col = pos - start_of_line.back();
                    tokens.emplace_back(floating_point_t(fpval, width), line, start_col, last_col);
                } else {
                    // integral

                    if (p >= input_length || c == ' ' || c == ')' || c == ',' || c == ';' ||
                        c == ' ' || c == '\n' || c == '\r' || c == ':') {
                        // if (p < input_length)
                        // cout << "SEPARATOR FOUND: \"" << c << "\"" << endl;
                        goto emit_integral_token;
                    } else {
                        // if (p < input_length)
                        // cout << "SEPARATOR NOT FOUND: \"" << c << "\"" << endl;
                    }

                    if (c == 'u') {
                        signedness = 'u';
                        ++p;
                    } else if (c == 'i') {
                        signedness = 'i';
                        ++p;
                    } else if (p < input_length) {
                        auto s = stringstream();
                        s << "Bad integral literal: \"" << c << "\"" << endl;
                        throw std::runtime_error(s.str());
                    }

                    if (!signedness) {
                        signedness = 'i';
                        width = 64;
                    } else {
                        if (p >= input_length) throw std::runtime_error("Fuckinng bad int.");

                        s = remaining_input(p);
                        if (s.size() >= 1 && s[0] == '8') {
                            width = 8;
                            ++p;
                        } else if (s.size() >= 2 && s.substr(0, 2) == "16"sv) {
                            width = 16;
                            p += 2;
                        } else if (s.size() >= 2 && s.substr(0, 2) == "32"sv) {
                            width = 32;
                            p += 2;
                        } else if (s.size() >= 2 && s.substr(0, 2) == "64"sv) {
                            width = 64;
                            p += 2;
                        } else {
                            throw std::runtime_error("Invalid integral width speecifier.");
                        }
                    }

                emit_integral_token:

                    if (signedness == 0) signedness = 'i';

                    pos = p;
                    const auto last_col = pos - start_of_line.back();
                    tokens.emplace_back(integral_t(intval, signedness, width), line, start_col,
                                        last_col);
                }
                return true;
            }

            auto eat_identifier(uint32_t& pos) -> bool {
                auto c = input[pos];
                if (std::isalpha(c) || c == '_') {
                    const auto start_col = pos - start_of_line.back() + 1;
                    auto s = string();
                    do {
                        s += c;
                        c = input[++pos];
                    } while (std::isalnum(c) || c == '_');

                    const auto line = start_of_line.size();
                    const auto last_col = pos - start_of_line.back();

                    tokens.emplace_back(identifier_t(s), line, start_col, last_col);

                    return true;
                }

                return false;
            }

            auto eat_string_literal(uint32_t& pos) -> bool {
                if (input[pos] != '"') return false;

                bool escape = false;

                auto s = string();

                const auto start_col = pos - start_of_line.back();

                auto p = pos + 1;
                for (; p < input_length; ++p) {
                    const auto c = input[p];

                    if (!escape) {
                        if (c == '"') break;
                        if (c == '\\')
                            escape = true;
                        else
                            s += c;
                    } else {
                        switch (c) {
                        case '"': s += '"'; break;
                        case '\\': s += '\\'; break;
                        case 'n': s += '\n'; break;
                        case 't': s += '\t'; break;
                        default: throw std::runtime_error("Unknown escaped string literal.");
                        }
                        escape = false;
                    }
                }

                pos = std::min(p + 1, input_length);

                const auto line = start_of_line.size();
                const auto last_col = pos - start_of_line.back();

                tokens.emplace_back(string_token_t(s), line, start_col, last_col);

                return true;
            }

            auto eat_basic_token(uint32_t& pos) -> bool {
                // step 1. *At compile time*, sort the tokens in order of decreasing
                // token symbol length. We want to match "verbatim" before "==", and
                // "==" before "=", and so on.
                constexpr auto length_sorted_regular_tokens =
                    hana::sort(hana::filter(token::types,
                                            [](auto t_c) {
                                                using t = typename decltype(t_c)::type;
                                                return hana::bool_c<!t::token.empty()>;
                                            }),
                               [](auto t_c, auto u_c) {
                                   using t = typename decltype(t_c)::type;
                                   using u = typename decltype(u_c)::type;
                                   constexpr auto result = t::token.size() > u::token.size();
                                   return hana::bool_c<result>;
                               });

                // sanity check: LINE_END token is empty, so it isn't regular.
                static_assert(hana::find(length_sorted_regular_tokens,
                                         hana::type_c<token::line_end_t>) == hana::nothing);

                // step 2: following function, at least in theory, desugars into a
                // statically-known series of if-then-else tests -- as many if-then
                // constructs as there are token types. If any token type's symbol
                // matches the input, we break early and eat that token.
                // The reason the below decodes into a static set of if-then-else
                // stmts is that length_sorted_regular_tokens is *constexpr*.

                const auto impl = hana::fix([&](auto self, auto token_types) -> bool {
                    if constexpr (hana::is_empty(token_types))
                        return false;
                    else {
                        constexpr auto first = hana::front(token_types);
                        using token_t = typename decltype(first)::type;
                        constexpr auto token_v = token_t{};

                        constexpr auto reserved_word = token_v.is_reserved_word;

                        bool match = token_v.token == input.substr(pos, token_v.token.size());
                        auto end_pos = pos + token_v.token.size();
                        if (token_v.is_reserved_word && end_pos != input_length &&
                            std::isalnum(input[end_pos]))
                            match = false;

                        if (match) {
                            const auto line = start_of_line.size();

                            /*
                            auto prev_line_last_non_eol_char = start_of_line.back() - 2;
                            while (std::isspace(input[prev_line_last_non_eol_char]) )
                                prev_line_last_non_eol_char--;
                            */
                            auto column = pos - start_of_line.back();

                            tokens.emplace_back(token_v, line, column + 1,
                                                column + token_v.token.size() + 1);

                            pos = end_pos;

                            return true;
                        }

                        return self(hana::drop_front(token_types));
                    }
                });

                // step 3: apply it.
                return impl(length_sorted_regular_tokens);
            }

            auto pop_dedents(uint32_t pos) -> void {
                while (size(margins) > 1) {
                    if (get<bool>(margins.back())) {
                        const auto line = start_of_line.size();
                        const auto column = pos - start_of_line[line - 2];
                        tokens.emplace_back(CPAREN, line, column, column);
                    }
                    margins.pop_back();
                }
            }

            auto eat_eol(uint32_t& p) -> bool {
                const auto c = input[p];
                if (c == '\n') {
                    p++;
                    return true;
                }
                if (c == '\r' && input[p + 1] == '\n') {
                    p += 2;
                    return true;
                }
                return false;
            }

            inline auto eoi(uint32_t p) -> bool { return p >= input_length; }

            auto eat_multiline_sep(uint32_t& pos) -> void {
                const auto s = remaining_input(pos).substr(0, 2);

                const auto line = start_of_line.size();
                const auto column = start_of_line.back() - 1;

                if (s == "--") {
                    if (tokens.back() == LINE_END)
                        tokens.back() = source_token_t(CPAREN, line, column, column);
                    else
                        tokens.emplace_back(CPAREN, line, column, column);
                    tokens.emplace_back(LINE_END, line, column, column);
                    tokens.emplace_back(OPAREN, line, column, column);
                    pos += 2;
                } else if (s == "..") {
                    if (!tokens.empty() && (tokens.back() == LINE_END ||
                                            tokens.back() == SEMICOLON || tokens.back() == COMMA))
                        tokens.pop_back();
                    pos += 2;
                }
            }

            auto eat_margin(uint32_t& pos) -> void {
                const auto s = remaining_input(pos);

                const auto n_spaces = s.find_first_not_of(' ');
                const auto [margin, real_indent] = margins.back();

                const auto line = start_of_line.size();
                const auto column = start_of_line.back() - 1;

                const auto colon_indent = !tokens.empty() && tokens.back() == COLON;
                const auto assign_indent = !tokens.empty() && tokens.back() == ASSIGN;

                if (n_spaces == margin) {
                    if (colon_indent) throw runtime_error("Indent expected");
                    if (!tokens.empty() && tokens.back().token != OPAREN)
                        tokens.emplace_back(LINE_END, line, column, column);
                } else if (n_spaces > margin) {
                    if (colon_indent)
                        tokens.back() = source_token_t(OPAREN, line, column, column);
                    else if (assign_indent)
                        tokens.emplace_back(OPAREN, line, column, column);
                    margins.emplace_back(n_spaces, colon_indent || assign_indent);
                } else {
                    if (colon_indent) throw runtime_error("Indent expected");

                    // auto all_ws = true;
                    while (!empty(margins) && n_spaces < get<int16_t>(margins.back())) {
                        if (get<bool>(margins.back()))
                            tokens.emplace_back(CPAREN, line, column, column);
                        // else all_ws = false;

                        margins.pop_back();
                    }

                    // if (all_ws) tokens.emplace_back(LINE_END, line, column, column);
                    tokens.emplace_back(LINE_END, line, column, column);
                }
                pos += n_spaces;
            }

            auto eat_empty_line(uint32_t& pos) -> bool {
                for (auto p = pos; p < input_length; p++) {
                    if (eat_eol(p)) {
                        pos = p;
                        return true;
                    }
                    const auto c = input[p];
                    if (c == '\t') throw std::runtime_error("Tabs are manifestly not allowed.");
                    if (c != ' ') return false;
                }
                pos = input_length;
                return true;
            }
        };
    }  // namespace

    namespace details {
        auto tokenize(string_view input) -> output_t {
            auto tokenizer = tokenizer_t(input);
            return tokenizer.process();
        }
    }  // namespace details
}}     // namespace bt::lexer
