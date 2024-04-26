from __future__ import annotations

import sys
import typing as t

from setuptools import find_packages, setup

project_base_url = 'https://github.com/lycantropos/cfractions/'
parameters: dict[str, t.Any] = dict(
    packages=find_packages(exclude=('tests', 'tests.*')),
    url=project_base_url,
    download_url=project_base_url + 'archive/master.zip',
)
if sys.implementation.name == 'cpython':
    from glob import glob

    from setuptools import Extension
    from setuptools.command.build_ext import build_ext

    class BuildExt(build_ext):
        def build_extensions(self) -> None:
            compile_args = []
            compiler_type = self.compiler.compiler_type
            if compiler_type == 'unix':
                compile_args.append('-Werror')
                compile_args.append('-Wall')
                compile_args.append('-Wextra')
                if sys.version_info < (3, 12):
                    # Python3.12 introduces conversion warnings
                    compile_args.append('-Wconversion')
            elif compiler_type == 'msvc':
                compile_args.append('/WX')
                compile_args.append('/W3')
            for extension in self.extensions:
                extension.extra_compile_args += compile_args
            super().build_extensions()

    parameters.update(
        cmdclass={build_ext.__name__: BuildExt},
        ext_modules=[Extension('cfractions._cfractions', glob('src/*.c'))],
        zip_safe=False,
    )
setup(**parameters)
