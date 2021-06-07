cfractions
==========

[![](https://dev.azure.com/lycantropos/cfractions/_apis/build/status/lycantropos.cfractions?branchName=master)](https://dev.azure.com/lycantropos/cfractions/_build/latest?definitionId=36&branchName=master "Azure Pipelines")
[![](https://codecov.io/gh/lycantropos/cfractions/branch/master/graph/badge.svg)](https://codecov.io/gh/lycantropos/cfractions "Codecov")
[![](https://img.shields.io/github/license/lycantropos/cfractions.svg)](https://github.com/lycantropos/cfractions/blob/master/LICENSE "License")
[![](https://badge.fury.io/py/cfractions.svg)](https://badge.fury.io/py/cfractions "PyPI")

Summary
-------

`cfractions` is a drop-in replacement for [`fractions` module](https://docs.python.org/library/fractions.html)
written using [`Python C API`](https://docs.python.org/c-api/index.html).

Main features are:
- speed & memory efficiency compared to pure-`Python` counterpart,
- full spectre of arithmetic & comparison operations,
- `Python3.5+` support,
- `PyPy` support (by falling back to `fractions.Fraction` proxy).

---

In what follows `python` is an alias for `python3.5` or `pypy3.5`
or any later version (`python3.6`, `pypy3.6` and so on).

Installation
------------

Install the latest `pip` & `setuptools` packages versions
```bash
python -m pip install --upgrade pip setuptools
```

### User

Download and install the latest stable version from `PyPI` repository
```bash
python -m pip install --upgrade cfractions
```

### Developer

Download the latest version from `GitHub` repository
```bash
git clone https://github.com/lycantropos/cfractions.git
cd cfractions
```

Install
```bash
python setup.py install
```

Usage
-----
```python
>>> from cfractions import Fraction
>>> Fraction()
Fraction(0, 1)
>>> Fraction(1, 2)
Fraction(1, 2)
>>> Fraction(50, 100)
Fraction(1, 2)
>>> Fraction(0.5)
Fraction(1, 2)
>>> Fraction(1, 3) + Fraction(1, 6)
Fraction(1, 2)
>>> Fraction(3, 2) - 1
Fraction(1, 2)
>>> 1 - Fraction(1, 2)
Fraction(1, 2)
>>> Fraction(1, 3) * Fraction(3, 2)
Fraction(1, 2)
>>> Fraction(1, 3) / Fraction(2, 3)
Fraction(1, 2)
>>> Fraction(1, 6) * 3
Fraction(1, 2)
>>> Fraction(3, 2) / 3
Fraction(1, 2)
>>> str(Fraction(1, 2))
'1/2'

```

Development
-----------

### Bumping version

#### Preparation

Install
[bump2version](https://github.com/c4urself/bump2version#installation).

#### Pre-release

Choose which version number category to bump following [semver
specification](http://semver.org/).

Test bumping version
```bash
bump2version --dry-run --verbose $CATEGORY
```

where `$CATEGORY` is the target version number category name, possible
values are `patch`/`minor`/`major`.

Bump version
```bash
bump2version --verbose $CATEGORY
```

This will set version to `major.minor.patch-alpha`. 

#### Release

Test bumping version
```bash
bump2version --dry-run --verbose release
```

Bump version
```bash
bump2version --verbose release
```

This will set version to `major.minor.patch`.

### Running tests

Install dependencies
```bash
python -m pip install -r requirements-tests.txt
```

Plain
```bash
pytest
```

Inside `Docker` container:
- with `CPython`
  ```bash
  docker-compose --file docker-compose.cpython.yml up
  ```
- with `PyPy`
  ```bash
  docker-compose --file docker-compose.pypy.yml up
  ```

`Bash` script (e.g. can be used in `Git` hooks):
- with `CPython`
  ```bash
  ./run-tests.sh
  ```
  or
  ```bash
  ./run-tests.sh cpython
  ```

- with `PyPy`
  ```bash
  ./run-tests.sh pypy
  ```

`PowerShell` script (e.g. can be used in `Git` hooks):
- with `CPython`
  ```powershell
  .\run-tests.ps1
  ```
  or
  ```powershell
  .\run-tests.ps1 cpython
  ```
- with `PyPy`
  ```powershell
  .\run-tests.ps1 pypy
  ```
