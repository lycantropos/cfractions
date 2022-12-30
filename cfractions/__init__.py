"""Python C API alternative to `fractions` module."""

__version__ = '2.2.0'

try:
    from ._cfractions import Fraction
except ImportError:
    import numbers as _numbers
    from fractions import Fraction as _Fraction


    class Fraction(_Fraction):
        def limit_denominator(self, max_denominator=10 ** 6):
            result = super().limit_denominator(max_denominator)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def as_integer_ratio(self):
            return self.numerator, self.denominator

        def __new__(cls, numerator=0, denominator=None):
            if denominator is not None:
                if not isinstance(denominator, int):
                    raise TypeError('Denominator should be an integer.')
                if not isinstance(numerator, int):
                    raise TypeError('Numerator should be an integer '
                                    'when denominator is specified.')
            return super().__new__(cls, numerator, denominator)

        def __abs__(self):
            result = super().__abs__()
            return Fraction(result.numerator, result.denominator)

        def __add__(self, other):
            result = super().__add__(_Fraction(other)
                                     if isinstance(other, _numbers.Rational)
                                     else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __copy__(self):
            cls = type(self)
            return (self
                    if cls is Fraction
                    else cls(self.numerator, self.denominator))

        def __deepcopy__(self, memo=None):
            return self.__copy__()

        def __divmod__(self, other):
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

        def __mul__(self, other):
            result = super().__mul__(_Fraction(other)
                                     if isinstance(other, _numbers.Rational)
                                     else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __neg__(self):
            result = super().__neg__()
            return Fraction(result.numerator, result.denominator)

        def __pos__(self):
            result = super().__pos__()
            return Fraction(result.numerator, result.denominator)

        def __pow__(self, exponent):
            result = super().__pow__(_Fraction(exponent.numerator,
                                               exponent.denominator)
                                     if isinstance(exponent, _numbers.Rational)
                                     else exponent)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __radd__(self, other):
            result = super().__radd__(_Fraction(other)
                                      if isinstance(other, _numbers.Rational)
                                      else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __rdivmod__(self, other):
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

        def __rmod__(self, other):
            result = (other % float(self)
                      if isinstance(other, float)
                      else super().__rmod__(other))
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __rmul__(self, other):
            result = super().__rmul__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __round__(self, precision=None):
            result = super().__round__(precision)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __rpow__(self, base):
            result = (_Fraction(base.numerator, base.denominator).__pow__(self)
                      if isinstance(base, _numbers.Rational)
                      else super().__rpow__(base))
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __rsub__(self, other):
            result = super().__rsub__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __rtruediv__(self, other):
            result = super().__rtruediv__(other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __sub__(self, other):
            result = super().__sub__(_Fraction(other)
                                     if isinstance(other, _numbers.Rational)
                                     else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)

        def __truediv__(self, other):
            result = super().__truediv__(_Fraction(other)
                                         if isinstance(other,
                                                       _numbers.Rational)
                                         else other)
            return (Fraction(result.numerator, result.denominator)
                    if isinstance(result, _Fraction)
                    else result)
