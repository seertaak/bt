class SymbolTable(dict):
    def __init__(self):
        self.bindings = dict()
        self.aux_stack = [None]

    def lookup(self, name):
        return self.bindings[name]

    def bind(self, name, value):
        if name in self.bindings:
            old_value = self.bindings[name]
            self.aux_stack.append((name, old_value))
        self.bindings[name] = value

    def exit(self):
        while self.aux_stack[-1]:
            name, value = self.aux_stack.pop()
            self.bindings[name] = value
        self.aux_stack.pop()

    def __exit__(self, type, value, traceback):
        self.exit()

    def enter(self):
        self.aux_stack.append(None)

    def __enter__(self):
        self.enter()
        return self

    def __repr__(self):
        return str(self.bindings)

    def str(self): return self.__repr__()

    
if __name__ == '__main__':
    tab = SymbolTable()

    tab.bind('foo', 1)
    tab.bind('bar', 2)

    print(tab)

    with tab:
        tab.bind('foo', 'oooh')
        print(tab)

    print(tab)

