from box import Box

some_const = True
some_string_const = "Bar"
some_map_const = {'foo': 'bar'}
some_set_const = {1, 2, 3}
some_array_const = [1, 2, 3]

def foo():
    ret = Box()
    ret.bar = 1
    return ret

def bar():
    ret = Box()
    if some_const:
        ret.bar = 2
    else:
        ret.bar = 57
    return ret
