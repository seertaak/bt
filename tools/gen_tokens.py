# Generate tokens

from jinja2 import Template

def_class = Template("""
        struct {{ class_name }} : token_tag {
            static constexpr const std::string_view name{"{{ name }}"};
            static constexpr const std::string_view token{"{{ symbol }}"};
        };""");

def_token = Template("""
    constexpr token_t {{ token }} {token::{{ token_type }}{}};""")

def run():
    template = Template(open('token.hpp.j2').read())

    tab = []
    with open('tokens.csv', 'rt') as f:
        for line in f.read().splitlines():
            parts = line.split(',')
            name = parts[0]
            symbol = "" if len(parts) == 1 else parts[1]
            tab.append((name, symbol))

    tab.sort(key=lambda p: (-len(p[1]), p[0]))

    class_defs, def_tokens = [], []

    for name, symbol in tab:
        class_defs.append(def_class.render(
                class_name="{}_t".format(name), 
                name=name.upper(),
                symbol=symbol))

        def_tokens.append(def_token.render(
                token=name.upper(),
                token_type="{}_t".format(name)))

    with open('tokens.hpp', 'w') as f:
        out = template.render(
                token_class_defs="".join(class_defs),
                tokens="".join(def_tokens))

        f.write(out)

if __name__ == '__main__':
    run()
