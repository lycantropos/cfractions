from __future__ import annotations

import numbers as _numbers
import typing as _t
from fractions import Fraction as _Fraction

import typing_extensions as _te

_Rational = _t.Union[int, _Fraction]


@_t.final
class Fraction(_numbers.Rational):
    @property
    def numerator(self, /) -> int:
        return self._value.numerator

    @property
    def denominator(self, /) -> int:
        return self._value.denominator

    def as_integer_ratio(self, /) -> tuple[int, int]:
        return self.numerator, self.denominator

    def limit_denominator(self, max_denominator: int = 10**6, /) -> _te.Self:
        result = self._value.limit_denominator(max_denominator)
        return (
            Fraction(result.numerator, result.denominator)
            if isinstance(result, _Fraction)
            else result
        )

    __module__ = 'cfractions'
    __slots__ = ('_value',)

    _value: _Fraction

    def __init_subclass__(cls, /, **_kwargs: _t.Any) -> None:
        raise TypeError(
            "type 'cfractions.Fraction' is not an acceptable base type"
        )

    @_t.overload
    def __new__(
        cls, value: _Rational | _te.Self | float | str = ..., _: None = ..., /
    ) -> _te.Self: ...

    @_t.overload
    def __new__(cls, numerator: int, denominator: int, /) -> _te.Self: ...

    def __new__(
        cls,
        numerator: _Rational | _te.Self | float | str = 0,
        denominator: int | None = None,
        /,
    ) -> _te.Self:
        if denominator is None:
            value = _Fraction(numerator)
        else:
            if not isinstance(denominator, int):
                raise TypeError('Denominator should be an integer.')
            if not isinstance(numerator, int):
                raise TypeError(
                    'Numerator should be an integer '
                    'when denominator is specified.'
                )
            value = _Fraction(numerator, denominator)
        self = super().__new__(cls)
        self._value = value
        return self

    def __abs__(self, /) -> _te.Self:
        return Fraction(abs(self._value))

    @_t.overload
    def __add__(self, other: _Rational | _te.Self, /) -> _te.Self: ...

    @_t.overload
    def __add__(self, other: float, /) -> float: ...

    @_t.overload
    def __add__(self, other: _t.Any, /) -> _t.Any: ...

    def __add__(self, other: _t.Any, /) -> _t.Any:
        result = self._value + (
            _Fraction(other) if isinstance(other, _numbers.Rational) else other
        )
        return Fraction(result) if isinstance(result, _Fraction) else result

    def __ceil__(self, /) -> int:
        return self._value.__ceil__()

    def __copy__(self, /) -> _te.Self:
        return self

    @_t.overload
    def __divmod__(
        self, other: _Rational | _te.Self, /
    ) -> tuple[int, _te.Self]: ...

    @_t.overload
    def __divmod__(self, other: float, /) -> tuple[float, float]: ...

    @_t.overload
    def __divmod__(self, other: _t.Any, /) -> _t.Any: ...

    def __divmod__(self, other: _t.Any, /) -> _t.Any:
        result = divmod(
            self._value,
            (
                _Fraction(other)
                if isinstance(other, _numbers.Rational)
                else other
            ),
        )
        return result[0], (
            Fraction(result[1]) if isinstance(result, tuple) else result
        )

    @_t.overload
    def __eq__(self, other: _Rational | _te.Self | float, /) -> bool: ...

    @_t.overload
    def __eq__(self, other: _t.Any, /) -> _t.Any: ...

    def __eq__(self, other: _t.Any, /) -> _t.Any:
        return self._value == other

    def __float__(self, /) -> float:
        return float(self._value)

    def __floor__(self, /) -> int:
        return self._value.__floor__()

    @_t.overload
    def __floordiv__(self, other: _Rational | _te.Self, /) -> int: ...

    @_t.overload
    def __floordiv__(self, other: float, /) -> float: ...

    @_t.overload
    def __floordiv__(self, other: _t.Any, /) -> _t.Any: ...

    def __floordiv__(self, other: _t.Any, /) -> _t.Any:
        return self._value // (
            _Fraction(other) if isinstance(other, _numbers.Rational) else other
        )

    @_t.overload
    def __ge__(self, other: _Rational | _te.Self, /) -> bool: ...

    @_t.overload
    def __ge__(self, other: float, /) -> bool: ...

    @_t.overload
    def __ge__(self, other: _t.Any, /) -> _t.Any: ...

    def __ge__(self, other: _t.Any, /) -> _t.Any:
        return self._value >= other

    @_t.overload
    def __gt__(self, other: _Rational | _te.Self, /) -> bool: ...

    @_t.overload
    def __gt__(self, other: float, /) -> bool: ...

    @_t.overload
    def __gt__(self, other: _t.Any, /) -> _t.Any: ...

    def __gt__(self, other: _t.Any, /) -> _t.Any:
        return self._value > other

    def __hash__(self, /) -> int:
        return hash(self._value)

    @_t.overload
    def __le__(self, other: _Rational | _te.Self, /) -> bool: ...

    @_t.overload
    def __le__(self, other: float, /) -> bool: ...

    @_t.overload
    def __le__(self, other: _t.Any, /) -> _t.Any: ...

    def __le__(self, other: _t.Any, /) -> _t.Any:
        return self._value <= other

    @_t.overload
    def __lt__(self, other: _Rational | _te.Self, /) -> bool: ...

    @_t.overload
    def __lt__(self, other: float, /) -> bool: ...

    @_t.overload
    def __lt__(self, other: _t.Any, /) -> _t.Any: ...

    def __lt__(self, other: _t.Any, /) -> _t.Any:
        return self._value < other

    @_t.overload
    def __mod__(self, other: _Rational | _te.Self, /) -> _te.Self: ...

    @_t.overload
    def __mod__(self, other: float, /) -> float: ...

    @_t.overload
    def __mod__(self, other: _t.Any, /) -> _t.Any: ...

    def __mod__(self, other: _t.Any, /) -> _t.Any:
        result = self._value % (
            _Fraction(other) if isinstance(other, _numbers.Rational) else other
        )
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __mul__(self, other: _Rational | _te.Self, /) -> _te.Self: ...

    @_t.overload
    def __mul__(self, other: float, /) -> float: ...

    @_t.overload
    def __mul__(self, other: _t.Any, /) -> _t.Any: ...

    def __mul__(self, other: _t.Any, /) -> _t.Any:
        result = self._value * (
            _Fraction(other) if isinstance(other, _numbers.Rational) else other
        )
        return Fraction(result) if isinstance(result, _Fraction) else result

    def __neg__(self, /) -> _te.Self:
        return Fraction(-self._value)

    def __pos__(self, /) -> _te.Self:
        return Fraction(+self._value)

    @_t.overload
    def __pow__(self, exponent: int, /) -> _te.Self: ...

    @_t.overload
    def __pow__(
        self, exponent: _Rational | _te.Self, /
    ) -> _te.Self | float: ...

    @_t.overload
    def __pow__(self, exponent: float, /) -> float: ...

    @_t.overload
    def __pow__(self, exponent: _t.Any, /) -> _t.Any: ...

    def __pow__(self, exponent: _t.Any, /) -> _t.Any:
        result = self._value ** (
            _Fraction(exponent)
            if isinstance(exponent, _numbers.Rational)
            else exponent
        )
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __radd__(self, other: _Rational, /) -> _te.Self: ...

    @_t.overload
    def __radd__(self, other: float, /) -> float: ...

    @_t.overload
    def __radd__(self, other: _t.Any, /) -> _t.Any: ...

    def __radd__(self, other: _t.Any, /) -> _t.Any:
        result = other + self._value
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __rdivmod__(self, other: _Rational, /) -> tuple[int, _te.Self]: ...

    @_t.overload
    def __rdivmod__(self, other: float, /) -> tuple[float, float]: ...

    @_t.overload
    def __rdivmod__(self, other: _t.Any, /) -> _t.Any: ...

    def __rdivmod__(self, other: _t.Any, /) -> _t.Any:
        result = divmod(other, self._value)
        return result[0], (
            Fraction(result[1]) if isinstance(result, tuple) else result
        )

    def __repr__(self) -> str:
        return self._value.__repr__()

    @_t.overload
    def __rfloordiv__(self, other: _Rational, /) -> int: ...

    @_t.overload
    def __rfloordiv__(self, other: float, /) -> float: ...

    @_t.overload
    def __rfloordiv__(self, other: _t.Any, /) -> _t.Any: ...

    def __rfloordiv__(self, other: _t.Any, /) -> _t.Any:
        return other // self._value

    @_t.overload
    def __rmod__(self, other: _Rational, /) -> _te.Self: ...

    @_t.overload
    def __rmod__(self, other: float, /) -> float: ...

    def __rmod__(self, other: _Rational | float, /) -> _te.Self | float:
        result = other % self._value
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __rmul__(self, other: _Rational, /) -> _te.Self: ...

    @_t.overload
    def __rmul__(self, other: float, /) -> float: ...

    @_t.overload
    def __rmul__(self, other: _t.Any, /) -> _t.Any: ...

    def __rmul__(self, other: _t.Any, /) -> _t.Any:
        result = other * self._value
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __round__(self, precision: None = ..., /) -> int: ...

    @_t.overload
    def __round__(self, precision: int, /) -> _te.Self: ...

    def __round__(self, precision: int | None = None, /) -> int | _te.Self:
        result = round(self._value, precision)
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __rpow__(self, base: _Rational, /) -> _te.Self | float: ...

    @_t.overload
    def __rpow__(self, base: float, /) -> float: ...

    def __rpow__(self, base: _Rational | float, /) -> _te.Self | float:
        result = base**self._value
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __rsub__(self, other: _Rational, /) -> _te.Self: ...

    @_t.overload
    def __rsub__(self, other: float, /) -> float: ...

    def __rsub__(self, other: _Rational | float, /) -> _te.Self | float:
        result = other - self._value
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __rtruediv__(self, other: _Rational, /) -> _te.Self: ...

    @_t.overload
    def __rtruediv__(self, other: float, /) -> float: ...

    @_t.overload
    def __rtruediv__(self, other: _t.Any, /) -> _t.Any: ...

    def __rtruediv__(self, other: _t.Any, /) -> _t.Any:
        result = other / self._value
        return Fraction(result) if isinstance(result, _Fraction) else result

    def __str__(self) -> str:
        return self._value.__str__()

    @_t.overload
    def __sub__(self, other: _Rational | _te.Self, /) -> _te.Self: ...

    @_t.overload
    def __sub__(self, other: float, /) -> float: ...

    @_t.overload
    def __sub__(self, other: _t.Any, /) -> _t.Any: ...

    def __sub__(self, other: _t.Any, /) -> _t.Any:
        result = self._value - (
            _Fraction(other) if isinstance(other, _numbers.Rational) else other
        )
        return Fraction(result) if isinstance(result, _Fraction) else result

    @_t.overload
    def __truediv__(self, other: _Rational | _te.Self, /) -> _te.Self: ...

    @_t.overload
    def __truediv__(self, other: float, /) -> float: ...

    @_t.overload
    def __truediv__(self, other: _t.Any, /) -> _t.Any: ...

    def __truediv__(self, other: _t.Any, /) -> _t.Any:
        result = self._value / (
            _Fraction(other) if isinstance(other, _numbers.Rational) else other
        )
        return Fraction(result) if isinstance(result, _Fraction) else result

    def __trunc__(self, /) -> int:
        return self._value.__trunc__()
