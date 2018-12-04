from setuptools import setup, Extension

xswap_cpp_extension = Extension(
    'xswap._xswap_backend', sources=['xswap/xswap_backend.cpp', 'xswap/hash_table.cpp'],
)

setup(
    name='xswap',
    version='0.1',
    description='Python-wrapped C/C++ library for degree-preserving network randomization',
    ext_modules=[xswap_cpp_extension],
    packages=['xswap'],
)
