"""Python C API alternative to `fractions` module."""

__version__ = '0.0.0'

try:
    from _cfractions import Fraction
except ImportError:
    from fractions import Fraction as _Fraction


    class Fraction(_Fraction):
        def __abs__(self) -> 'Fraction':
            return Fraction(abs(self.numerator), self.denominator)

        def __neg__(self) -> 'Fraction':
            return Fraction(-self.numerator, self.denominator)
