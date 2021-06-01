"""Python C API alternative to `fractions` module."""

__version__ = '0.0.0'

try:
    from _cfractions import Fraction
except ImportError:
    import numbers as _numbers
    from fractions import Fraction as _Fraction
    from typing import (TypeVar as _TypeVar,
                        overload as _overload)

    _Number = _TypeVar('_Number',
                       bound=_numbers.Number)


    class Fraction(_Fraction):
        def __abs__(self) -> 'Fraction':
            return Fraction(abs(self.numerator), self.denominator)

        @_overload
        def __mul__(self, other: _numbers.Rational) -> 'Fraction':
            """Returns product of the fraction with given rational number."""

        @_overload
        def __mul__(self, other: _Number) -> _Number:
            """Returns product of the fraction with given number."""

        def __mul__(self, other):
            result = super().__mul__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __neg__(self) -> 'Fraction':
            return Fraction(-self.numerator, self.denominator)
