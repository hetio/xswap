import os
import pathlib
import re

import setuptools

os.environ["CC"] = "g++"

directory = pathlib.Path(__file__).parent.resolve()

# version
init_path = directory.joinpath('xswap', '__init__.py')
text = init_path.read_text()
pattern = re.compile(r"^__version__ = ['\"]([^'\"]*)['\"]", re.MULTILINE)
version = pattern.search(text).group(1)

# long_description
readme_path = directory.joinpath('README.md')
long_description = readme_path.read_text()

xswap_cpp_extension = setuptools.Extension(
    'xswap._xswap_backend',
    sources=['xswap/xswap_wrapper.cpp', 'xswap/hash_table.cpp', 'xswap/xswap.cpp'],
    extra_compile_args=["-std=c++11"],
)

setuptools.setup(
    # Package details
    name='xswap',
    version=version,
    url='https://github.com/greenelab/xswap',
    description='Python-wrapped C/C++ library for degree-preserving network randomization',
    long_description_content_type='text/markdown',
    long_description=long_description,
    license='BSD 2-Clause',

    # Author details
    author='Michael Zietz',
    author_email='michael.zietz@gmail.com',

    # Specify python version
    python_requires='>=3.5',

    ext_modules=[xswap_cpp_extension],
    packages=['xswap'],
)
