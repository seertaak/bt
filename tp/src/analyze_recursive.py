import ast
import numpy as np

from symbol_table import SymbolTable

from ast import (
    If, For, While, Try, TryFinally, TryExcept, 
    FunctionDef, ClassDef, AsyncFunctionDef, AsyncFor, 
    ExceptHandler, With, Lambda, AsyncWith, 
)

def annotate_recursive_functions(module):
    calls = {}
    called_fns = []


    class Detector(ast.NodeVisitor):
        def __init__(self):
            self.symtab = SymbolTable()
            self.scope_types = {
                If, For, While, Try, TryFinally, TryExcept, 
                FunctionDef, ClassDef, AsyncFunctionDef, AsyncFor, 
                ExceptHandler, With, Lambda, AsyncWith
            }

        def visit_generic(self, node):
            if type(node) in scope_types:
                with self.symtab:
                    ast.NodeVisitor.generic_visit(self, node)
                    
                
        def visit_Call(self, node): 
            self.interp_call = self.interp_call or (
                    isinstance(node.func, ast.Attribute)     
                and isinstance(node.func.value, ast.Name) 
                and node.func.value.id == 'interp'
            )
            return ast.NodeTransformer.generic_visit(self, node)

    Detector().visit(module)
    
    n = len(calls.keys())
    E = np.zeros((n, n))
    return (calls.keys(), E)


