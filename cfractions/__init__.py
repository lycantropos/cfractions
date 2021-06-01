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
            result = super().__abs__()
            return Fraction(result.numerator, result.denominator)

        @_overload
        def __add__(self, other: _numbers.Rational) -> 'Fraction':
            """Returns sum of the fraction with given rational number."""

        @_overload
        def __add__(self, other: _Number) -> _Number:
            """Returns sum of the fraction with given number."""

        def __add__(self, other):
            result = super().__add__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        @_overload
        def __mod__(self, other: _numbers.Rational) -> 'Fraction':
            """
            Returns remainder of division of the fraction
            by given rational number.
            """

        @_overload
        def __mod__(self, other: _Number) -> _Number:
            """
            Returns remainder of division of the fraction by given number.
            """

        def __mod__(self, other):
            result = super().__mod__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

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
            result = super().__neg__()
            return Fraction(result.numerator, result.denominator)

        def __pos__(self) -> 'Fraction':
            result = super().__pos__()
            return Fraction(result.numerator, result.denominator)

        @_overload
        def __radd__(self, other: _numbers.Rational) -> 'Fraction':
            """Returns sum of given rational number with the fraction."""

        @_overload
        def __radd__(self, other: _Number) -> _Number:
            """Returns sum of given number with the fraction."""

        def __radd__(self, other):
            result = super().__radd__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        @_overload
        def __rmul__(self, other: _numbers.Rational) -> 'Fraction':
            """Returns product of given rational number with the fraction."""

        @_overload
        def __rmul__(self, other: _Number) -> _Number:
            """Returns product of given number with the fraction."""

        def __rmul__(self, other):
            result = super().__rmul__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        @_overload
        def __rsub__(self, other: _numbers.Rational) -> 'Fraction':
            """
            Returns difference of given rational number with the fraction.
            """

        @_overload
        def __rsub__(self, other: _Number) -> _Number:
            """Returns difference of given number with the fraction."""

        def __rsub__(self, other):
            result = super().__rsub__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        @_overload
        def __rtruediv__(self, other: _numbers.Rational) -> 'Fraction':
            """Returns division of given rational number by the fraction."""

        @_overload
        def __rtruediv__(self, other: _Number) -> _Number:
            """Returns division of given number by the fraction."""

        def __rtruediv__(self, other):
            result = super().__rtruediv__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        @_overload
        def __sub__(self, other: _numbers.Rational) -> 'Fraction':
            """
            Returns difference of the fraction with given rational number.
            """

        @_overload
        def __sub__(self, other: _Number) -> _Number:
            """Returns difference of the fraction with given number."""

        def __sub__(self, other):
            result = super().__sub__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        @_overload
        def __truediv__(self, other: _numbers.Rational) -> 'Fraction':
            """Returns division of the fraction by given rational number."""

        @_overload
        def __truediv__(self, other: _Number) -> _Number:
            """Returns division of the fraction by given number."""

        def __truediv__(self, other):
            result = super().__truediv__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)
