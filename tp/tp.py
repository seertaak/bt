import ast
import astor
from astor.node_util import ExplicitNodeVisitor
import pudb
import numpy as np

import foo2

class IfSimplifier(ast.NodeTransformer):
    def __init__(self, values={}, nodes={}, imports=[]):
        self.values = values
        self.nodes = nodes
        self.imports = imports
        self.interp_call = False


    def evaluate(self, n):
        body = []
        body.extend(self.imports)
        k = "__RESULT__"
        a = ast.Assign(
            targets=[ast.Name(id=k, ctx=ast.Store())],
            value=n
        )
        body.append(a)
        m = ast.Module(body=body)
        ast.fix_missing_locations(m)
        bytecode = compile(m, "<ast>", 'exec')
        exec(bytecode, self.values)
        result = self.values[k]
        del self.values[k]
        return result


    def visit_ImportFrom(self, node):
        self.imports.append(node)
        return ast.NodeTransformer.generic_visit(self, node)


    def visit_FunctionDef(self, node):
        self.interp_call = False
        result = ast.NodeTransformer.generic_visit(self, node)
        result.interp_call = self.interp_call
        print(f"FunctionDef: {result.name} has interp calls: {result.interp_call}")
        return result


    def visit_Call(self, node): 
        self.interp_call = self.interp_call or (
                isinstance(node.func, ast.Attribute)     
            and isinstance(node.func.value, ast.Name) 
            and node.func.value.id == 'interp'
        )
        return ast.NodeTransformer.generic_visit(self, node)


    def visit_If(self, node):
        def successful_branch(n):
            if isinstance(n, ast.If): 
                if self.evaluate(n.test):
                    ifs = IfSimplifier(self.values, self.nodes, self.imports)
                    r = []
                    for e in n.body:
                        re = ifs.visit(e)
                        if isinstance(re, list):
                            r.extend(re)
                        else:
                            r.append(re)
                    return r
                elif not n.orelse:
                    return None
                else:
                    assert len(n.orelse) == 1
                    return successful_branch(n.orelse[0])
            return n
                
        return successful_branch(node)
        

    def visit_Assign(self, node):
        def interp_call(rhs):
            return isinstance(rhs, ast.Call)                \
               and isinstance(rhs.func, ast.Attribute)      \
               and isinstance(rhs.func.value, ast.Name)     \
               and rhs.func.value.id == 'interp'

        def fqn(node):
            if isinstance(node, ast.Name):
                return node.id
            elif isinstance(node, ast.Attribute):
                name = fqn(node.value)
                return f"{name}.{node.attr}"


        rhs = node.value
        if interp_call(rhs):
            assert len(node.targets) == 1
            lhs = fqn(node.targets[0])
            self.nodes[lhs] = ast.dump(rhs)
        else:
            assert len(node.targets) == 1
            n = fqn(node.targets[0])

            if n != 'interp':
                body = []
                body.append(ast.ImportFrom(module='box', names=[
                    ast.alias(name='Box', asname=None)
                ]))
                body.append(node)
                m = ast.Module(body=[
                    ast.ImportFrom(module='box', names=[
                        ast.alias(name='Box', asname=None)
                    ]),
                    node,
                ])
                ast.fix_missing_locations(m)
                rhs_bytecode = compile(m, "<ast>", 'exec')
                exec(rhs_bytecode, self.values)

        return ast.NodeTransformer.generic_visit(self, node)


def fqn(node):
    if isinstance(node, ast.Name):
        return node.id
    elif isinstance(node, ast.Attribute):
        name = fqn(node.value)
        return f"{name}.{node.attr}"


def annotate_recursive_functions(module):
    calls = {}
    called_fns = []

    class Detector(ast.NodeVisitor):
        def __init__(self):
            self.calls = {}
            self.called_fns = []
            self.caller_path = []

        def visit_ClassDef(self, node):
            self.caller_path.append(node.name)
            ast.NodeVisitor.generic_visit(self, node)
            self.caller_path.pop()
            
        def visit_FunctionDef(self, node): 
            self.caller_path.append(node.name)
            called_fns = []
            ast.NodeVisitor.generic_visit(self, node)
            calls['.'.join(self.caller_path)] = called_fns
            self.caller_path.pop()

        def visit_Call(self, node): 
            called_fns.append(fqn(node.func))
            ast.NodeVisitor.generic_visit(self, node)

    Detector().visit(module)
    
    n = len(calls.keys())
    E = np.zeros((n, n))
    return (calls.keys(), E)


class EndCalculator(ast.NodeVisitor):
    def __init__(self):
        self.values = {}
        self.nodes = {}
        self.imports = []


    def evaluate(self, n):
        body = []
        body.extend(self.imports)
        k = "__RESULT__"
        a = ast.Assign(
            targets=[ast.Name(id=k, ctx=ast.Store())],
            value=n
        )
        body.append(a)
        m = ast.Module(body=body)
        ast.fix_missing_locations(m)
        bytecode = compile(m, "<ast>", 'exec')
        exec(bytecode, self.values)
        result = self.values[k]
        del self.values[k]
        return result


    def visit_ImportFrom(self, node):
        self.imports.append(node)
        ast.NodeVisitor.generic_visit(self, node)


    def visit_If(self, node):
        r = self.evaluate(node.test)
        print(f"{node.test}={r}")
        print(node.body)
        print(node.orelse)
        ast.NodeVisitor.generic_visit(self, node)


    def visit_Assign(self, node):
        def interp_call(rhs):
            return isinstance(rhs, ast.Call)        \
               and isinstance(rhs.func, ast.Attribute)    \
               and isinstance(rhs.func.value, ast.Name)   \
               and rhs.func.value.id == 'interp'

        def fqn(node):
            if isinstance(node, ast.Name):
                return node.id
            elif isinstance(node, ast.Attribute):
                name = fqn(node.value)
                return f"{name}.{node.attr}"


        rhs = node.value
        if interp_call(rhs):
            assert len(node.targets) == 1
            lhs = fqn(node.targets[0])
            self.nodes[lhs] = ast.dump(rhs)
        else:
            assert len(node.targets) == 1
            n = fqn(node.targets[0])

            if n != 'interp':
                body = []
                body.append(ast.ImportFrom(module='box', names=[
                    ast.alias(name='Box', asname=None)
                ]))
                body.append(node)
                m = ast.Module(body=[
                    ast.ImportFrom(module='box', names=[
                        ast.alias(name='Box', asname=None)
                    ]),
                    node,
                ])
                ast.fix_missing_locations(m)
                rhs_bytecode = compile(m, "<ast>", 'exec')
                exec(rhs_bytecode, self.values)

        ast.NodeVisitor.generic_visit(self, node)


#T = IfSimplifier()
tree = astor.code_to_ast(foo2)
#tree = T.visit(tree)

print(annotate_recursive_functions(tree))

#print(f"Nodes :\n{T.nodes}")
#print(f"Values:\n{T.values}")
#print(astor.code_gen.to_source(tree))
#print(astor.dump_tree(tree))
