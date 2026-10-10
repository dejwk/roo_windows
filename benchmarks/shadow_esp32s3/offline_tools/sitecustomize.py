"""Keep this benchmark on the already installed pioarduino Python dependencies.
Loaded only when explicitly put on PYTHONPATH. No installed files are changed.
"""
from importlib.machinery import SourceFileLoader
original_exec = SourceFileLoader.exec_module

def execute_without_dependency_upgrade(loader, module):
    original_exec(loader, module)
    if module.__name__ == 'penv_setup':
        module.install_python_deps = lambda *args, **kwargs: True
        module.has_internet_connection = lambda *args, **kwargs: False
SourceFileLoader.exec_module = execute_without_dependency_upgrade
