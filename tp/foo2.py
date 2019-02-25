from box import Box

a = 2
b = a + 2

x = Box(default_box=True)
x.y.z = 2
x.y.y = x.y.z + 2

class Interp(object):
    def invoke(self): pass
    def __getattr__(self, key): return self.invoke

interp = Interp()

def recursive_fn():
    return 1 + recursive_fn()

def invert():
    def inner_interp():
        n1 = interp.bing()
        return n1
        
    return 45


def simple_foo(arg):
    ret = Box(default_box=True)
    if arg == 1:
        ret.test = 1
    elif arg == 2:
        ret = simple_foo(1)
        ret.zztop = 5

    return ret
        
    


def bar():
    def no_interp():
        pass

    bingo = interp.foo()

    if x.y.z == 2:
        n1 = interp.bing()
        if x.y.y == 3:
            n2 = interp.bingo()
        elif x.y.y == x.y.z + 2:
            n2 = interp.bilbo(n2)
            n3 = interp.balaal()
            if b == a + 2:
                n4= interp.halal()
        elif x.y.y == x.y.z + 5:
            n5 = interp.bango()
            ny= interp.harbo()
        else:
            n34 = interp.fango()

    else:
        n234234 = interp.bang()

