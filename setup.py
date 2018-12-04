from distutils.core import setup, Extension

module1 = Extension('xswap._xswap_backend',
                    sources=['xswap/py_xswap.cpp', 'xswap/hash_table.cpp'],
                    )

setup(
    name='xswap',
    version='1.0',
    description='This is a demo package',
    ext_modules=[module1]
)
