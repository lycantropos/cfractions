"""Python C API alternative to `fractions` module."""

__version__ = '1.4.0'

try:
    from _cfractions import Fraction
except ImportError:
    import numbers as _numbers
    from fractions import Fraction as _Fraction
    from typing import (Any as _Any,
                        Dict as _Dict,
                        Optional as _Optional,
                        Tuple as _Tuple,
                        TypeVar as _TypeVar,
                        Union as _Union,
                        overload as _overload)

    _Number = _TypeVar('_Number',
                       bound=_numbers.Number)


    class Fraction(_Fraction):
        def limit_denominator(self, max_denominator: int = 10 ** 6
                              ) -> 'Fraction':
            result = super().limit_denominator(max_denominator)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def as_integer_ratio(self) -> _Tuple[int, int]:
            return self.numerator, self.denominator

        def __new__(cls,
                    numerator: _Union[int, float] = 0,
                    denominator: _Optional[int] = None,
                    **kwargs) -> 'Fraction':
            if denominator is not None:
                if not isinstance(denominator, int):
                    raise TypeError('Denominator should be an integer.')
                if not isinstance(numerator, int):
                    raise TypeError('Numerator should be an integer '
                                    'when denominator is specified.')
            return super().__new__(cls, numerator, denominator, **kwargs)

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
            result = super().__add__(_Fraction(other)
                                     if isinstance(other, _numbers.Rational)
                                     else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __copy__(self) -> 'Fraction':
            cls = type(self)
            return (self
                    if cls is Fraction
                    else cls(self._numerator, self._denominator))

        def __deepcopy__(self, memo: _Optional[_Dict[int, _Any]] = None
                         ) -> 'Fraction':
            return self.__copy__()

        def __divmod__(self, other: _Number
                       ) -> _Tuple[_Number, _Union['Fraction', _Number]]:
            result = (divmod(float(self), other)
                      if isinstance(other, float)
                      else super().__divmod__(_Fraction(other)
                                              if isinstance(other,
                                                            _numbers.Rational)
                                              else other))
            return ((result[0],
                     Fraction(result[1].numerator, result[1].denominator)
                     if isinstance(result[1], _numbers.Rational)
                     else result[1])
                    if isinstance(result, tuple)
                    else result)

        @_overload
        def __floordiv__(self, other: _numbers.Rational) -> 'Fraction':
            """
            Returns quotient of division of the fraction
            by given rational number.
            """

        @_overload
        def __floordiv__(self, other: _Number) -> _Number:
            """Returns quotient of division of the fraction by given number."""

        def __floordiv__(self, other):
            result = (float(self) // other
                      if isinstance(other, float)
                      else
                      super().__floordiv__(_Fraction(other)
                                           if isinstance(other,
                                                         _numbers.Rational)
                                           else other))
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
            result = (float(self) % other
                      if isinstance(other, float)
                      else
                      super().__mod__(_Fraction(other)
                                      if isinstance(other, _numbers.Rational)
                                      else other))
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
            result = super().__mul__(_Fraction(other)
                                     if isinstance(other, _numbers.Rational)
                                     else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __neg__(self) -> 'Fraction':
            result = super().__neg__()
            return Fraction(result.numerator, result.denominator)

        def __pos__(self) -> 'Fraction':
            result = super().__pos__()
            return Fraction(result.numerator, result.denominator)

        def __pow__(self,
                    exponent: _numbers.Complex,
                    modulo: _Optional[_numbers.Complex] = None
                    ) -> _numbers.Complex:
            result = super().__pow__(_Fraction(exponent.numerator,
                                               exponent.denominator)
                                     if isinstance(exponent, _numbers.Rational)
                                     else exponent)
            if isinstance(result, _numbers.Complex) and modulo is not None:
                result %= (_Fraction(modulo)
                           if isinstance(modulo, _numbers.Rational)
                           else modulo)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        @_overload
        def __radd__(self, other: _numbers.Rational) -> 'Fraction':
            """Returns sum of given rational number with the fraction."""

        @_overload
        def __radd__(self, other: _Number) -> _Number:
            """Returns sum of given number with the fraction."""

        def __radd__(self, other):
            result = super().__radd__(_Fraction(other)
                                      if isinstance(other, _numbers.Rational)
                                      else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __rdivmod__(self, other: _Number
                        ) -> _Tuple[_Number, _Union['Fraction', _Number]]:
            result = (divmod(other, float(self))
                      if isinstance(other, float)
                      else super().__rdivmod__(_Fraction(other)
                                               if isinstance(other,
                                                             _numbers.Rational)
                                               else other))
            return ((result[0],
                     Fraction(result[1].numerator, result[1].denominator)
                     if isinstance(result[1], _numbers.Rational)
                     else result[1])
                    if isinstance(result, tuple)
                    else result)

        @_overload
        def __rfloordiv__(self, other: _numbers.Rational) -> 'Fraction':
            """
            Returns quotient of division of given rational number
            by the fraction.
            """

        @_overload
        def __rfloordiv__(self, other: _Number) -> _Number:
            """Returns quotient of division of given number by the fraction."""

        def __rfloordiv__(self, other):
            result = (other // float(self)
                      if isinstance(other, float)
                      else
                      super().__rfloordiv__(Fraction(other)
                                            if isinstance(other,
                                                          _numbers.Rational)
                                            else other))
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        @_overload
        def __rmod__(self, other: _numbers.Rational) -> 'Fraction':
            """
            Returns remainder of division of given rational number
            by the fraction.
            """

        @_overload
        def __rmod__(self, other: _Number) -> _Number:
            """
            Returns remainder of division of given number by the fraction.
            """

        def __rmod__(self, other):
            result = (other % float(self)
                      if isinstance(other, float)
                      else super().__rmod__(other))
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

        def __round__(self, precision: _Optional[int] = None
                      ) -> _Union[int, 'Fraction']:
            result = super().__round__(precision)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __rpow__(self,
                     base: _numbers.Complex,
                     modulo: _Optional[_numbers.Complex] = None
                     ) -> _numbers.Complex:
            result = (_Fraction(base.numerator, base.denominator).__pow__(self)
                      if isinstance(base, _numbers.Rational)
                      else super().__rpow__(base))
            if isinstance(result, _numbers.Complex) and modulo is not None:
                result %= modulo
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
            result = super().__sub__(_Fraction(other)
                                     if isinstance(other, _numbers.Rational)
                                     else other)
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
            result = super().__truediv__(_Fraction(other)
                                         if isinstance(other,
                                                       _numbers.Rational)
                                         else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)
