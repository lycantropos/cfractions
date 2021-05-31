"""Python C API alternative to `fractions` module."""

__version__ = '0.0.0'

try:
    from _cfractions import Fraction
except ImportError:
    from fractions import Fraction
