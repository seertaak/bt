# Generate tokens

from jinja2 import Template
import sys
import string

def_class = Template("""
struct {{ class_name }} : token_tag {
    static constexpr const std::string_view name{"{{ name }}"};
    static constexpr const std::string_view token{"{{ symbol }}"};
    static constexpr const uint32_t categories = {{ categories }};
    static constexpr const bool is_reserved_word = {{ is_reserved_word }};
};""");

def_token = Template("""
const token_t {{ token }} {token::{{ token_type }}{}};""")

def_types_ns = Template("""token::{{ token_type }}""")
def_types = Template("""{{ token_type }}""")

def run():
    template = Template(open('token.hpp.j2').read())

    tab = []
    with open('tokens.csv', 'rt') as f:
        for line in f.read().splitlines():
            parts = line.split(',')
            name = parts[0]
            symbol = "" if len(parts) == 1 else parts[1]
            if name == 'comma': symbol = ','
            tab.append((name, symbol))

    tab.sort(key=lambda p: (-len(p[1]), p[0]))

    class_defs, def_tokens, token_type_list, token_type_list_ns = [], [], [], []

    for name, symbol in tab:
        is_reserved_word = False
        categories = []
        if len(symbol) == 0:
            categories.append('synthetic')
        elif symbol.lower().isalpha():
            categories.append('reserved_word')
            is_reserved_word = True
        elif symbol.lower()[0].isalpha() and symbol.lower()[1:].isalnum():
            categories.append('reserved_word')
            is_reserved_word = True
        elif symbol in "{}[]()":
            categories.append('grouping_token')
        elif symbol in r'.,;\\':
            categories.append('punctuation')

        if symbol:
            if symbol == 'not' or symbol in '~*&@#$':
                categories.append('unary_prefix_op')
            elif symbol in '?!':
                categories = ['unary_postfix_op']

            if symbol in '+,-,*,/,%,^,=,==,!=,<=,>=,<,>,and,not,or,xor,+=,-=,*=,/=,%=,^=,=>,->,`,|,|=,:,:*,:/,:%,>>,<<,**'.split(','):
                categories.append('binary_op')

        class_defs.append(def_class.render(
                class_name="{}_t".format(name), 
                name=name.upper(),
                symbol=symbol,
                categories=' | '.join(categories),
                is_reserved_word=str(is_reserved_word).lower()))

        token_type = "{}_t".format(name)

        token_type_list.append(def_types.render(token_type=token_type))
        token_type_list_ns.append(def_types_ns.render(token_type=token_type))

        def_tokens.append(def_token.render(
                token=name.upper(),
                token_type=token_type))

    with open('tokens.hpp', 'w') as f:
        out = template.render(
                token_class_defs="".join(class_defs),
                token_type_list=",".join(token_type_list),
                token_type_list_ns=",".join(token_type_list_ns),
                tokens="".join(def_tokens))

        f.write(out)

if __name__ == '__main__':
    run()
