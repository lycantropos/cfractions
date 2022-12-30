import platform
from pathlib import Path

from setuptools import (find_packages,
                        setup)

import cfractions

project_base_url = 'https://github.com/lycantropos/cfractions/'


def read_file(path_string: str) -> str:
    return Path(path_string).read_text(encoding='utf-8')


parameters = dict(
        name=cfractions.__name__,
        packages=find_packages(exclude=('tests', 'tests.*')),
        package_data={cfractions.__name__: ['py.typed', '__init__.pyi']},
        version=cfractions.__version__,
        description=cfractions.__doc__,
        long_description=read_file('README.md'),
        long_description_content_type='text/markdown',
        author='Azat Ibrakov',
        author_email='azatibrakov@gmail.com',
        license='MIT License',
        classifiers=[
            'License :: OSI Approved :: MIT License',
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: 3.8',
            'Programming Language :: Python :: 3.9',
            'Programming Language :: Python :: 3.10',
            'Programming Language :: Python :: 3.11',
            'Programming Language :: Python :: Implementation :: CPython',
            'Programming Language :: Python :: Implementation :: PyPy',
        ],
        url=project_base_url,
        download_url=project_base_url + 'archive/master.zip',
        python_requires='>=3.7'
)
if platform.python_implementation() == 'CPython':
    from glob import glob

    from setuptools import Extension

    parameters.update(ext_modules=[Extension(cfractions.__name__ + '._'
                                             + cfractions.__name__,
                                             glob('src/*.c'))],
                      zip_safe=False)
setup(**parameters)
