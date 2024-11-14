from __future__ import annotations

import numbers as _numbers
import sys
from fractions import Fraction as _Fraction
from typing import (
    Any as _Any,
    TYPE_CHECKING,
    Union as _Union,
    final as _final,
    overload as _overload,
)

if TYPE_CHECKING:
    from typing_extensions import Self

    _Rational = _Union[_Fraction, _numbers.Rational, int]


@_final
class Fraction(_numbers.Rational):
    @property
    def numerator(self, /) -> int:
        return self._value.numerator

    @property
    def denominator(self, /) -> int:
        return self._value.denominator

    def as_integer_ratio(self, /) -> tuple[int, int]:
        return self.numerator, self.denominator

    if sys.version_info >= (3, 12):

        def is_integer(self, /) -> bool:
            return self._value.is_integer()

    else:

        def is_integer(self, /) -> bool:
            return self.denominator == 1

    def limit_denominator(self, max_denominator: int = 10**6, /) -> Self:
        return _to_fraction_if_std_fraction(
            self._value.limit_denominator(max_denominator)
        )

    __module__ = 'cfractions'
    __slots__ = ('_value',)

    _value: _Fraction

    def __init_subclass__(cls, /, **_kwargs: _Any) -> None:
        raise TypeError(
            "type 'cfractions.Fraction' is not an acceptable base type"
        )

    @_overload
    def __new__(
        cls, value: _Rational | Self | float | str = ..., _: None = ..., /
    ) -> Self: ...

    @_overload
    def __new__(cls, numerator: int, denominator: int, /) -> Self: ...

    def __new__(
        cls,
        numerator: _Rational | Self | float | str = 0,
        denominator: int | None = None,
        /,
    ) -> Self:
        if denominator is None:
            value = (
                _Fraction(int(numerator.numerator), int(numerator.denominator))
                if isinstance(numerator, _numbers.Rational)
                else _Fraction(numerator)
            )
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

    def __abs__(self, /) -> Self:
        return Fraction(abs(self._value))

    @_overload
    def __add__(self, other: _Rational | Self, /) -> Self: ...

    @_overload
    def __add__(self, other: float, /) -> float: ...

    @_overload
    def __add__(self, other: _Any, /) -> _Any: ...

    def __add__(self, other: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            self._value + _to_std_fraction_if_rational(other)
        )

    def __ceil__(self, /) -> int:
        return self._value.__ceil__()

    def __copy__(self, /) -> Self:
        return self

    def __deepcopy__(self, memo: dict[str, _Any] | None = None, /) -> Self:
        return self

    @_overload
    def __divmod__(self, divisor: _Rational | Self, /) -> tuple[int, Self]: ...

    @_overload
    def __divmod__(self, divisor: float, /) -> tuple[float, float]: ...

    @_overload
    def __divmod__(self, divisor: _Any, /) -> _Any: ...

    def __divmod__(self, divisor: _Any, /) -> _Any:
        result = divmod(self._value, _to_std_fraction_if_rational(divisor))
        return (
            (result[0], Fraction(result[1]))
            if isinstance(result, tuple) and isinstance(result[1], _Fraction)
            else result
        )

    @_overload
    def __eq__(self, other: _Rational | Self | float, /) -> bool: ...

    @_overload
    def __eq__(self, other: _Any, /) -> _Any: ...

    def __eq__(self, other: _Any, /) -> _Any:
        return self._value == other

    def __float__(self, /) -> float:
        return float(self._value)

    def __floor__(self, /) -> int:
        return self._value.__floor__()

    @_overload
    def __floordiv__(self, divisor: _Rational | Self, /) -> int: ...

    @_overload
    def __floordiv__(self, divisor: float, /) -> float: ...

    @_overload
    def __floordiv__(self, divisor: _Any, /) -> _Any: ...

    def __floordiv__(self, divisor: _Any, /) -> _Any:
        return self._value // _to_std_fraction_if_rational(divisor)

    @_overload
    def __ge__(self, other: _Rational | Self, /) -> bool: ...

    @_overload
    def __ge__(self, other: float, /) -> bool: ...

    @_overload
    def __ge__(self, other: _Any, /) -> _Any: ...

    def __ge__(self, other: _Any, /) -> _Any:
        return self._value >= other

    @_overload
    def __gt__(self, other: _Rational | Self, /) -> bool: ...

    @_overload
    def __gt__(self, other: float, /) -> bool: ...

    @_overload
    def __gt__(self, other: _Any, /) -> _Any: ...

    def __gt__(self, other: _Any, /) -> _Any:
        return self._value > other

    def __hash__(self, /) -> int:
        return hash(self._value)

    if sys.version_info >= (3, 11):

        def __int__(self, /) -> int:
            return self._value.__int__()

    else:

        def __int__(self, /) -> int:
            return self.__trunc__()

    @_overload
    def __le__(self, other: _Rational | Self, /) -> bool: ...

    @_overload
    def __le__(self, other: float, /) -> bool: ...

    @_overload
    def __le__(self, other: _Any, /) -> _Any: ...

    def __le__(self, other: _Any, /) -> _Any:
        return self._value <= other

    @_overload
    def __lt__(self, other: _Rational | Self, /) -> bool: ...

    @_overload
    def __lt__(self, other: float, /) -> bool: ...

    @_overload
    def __lt__(self, other: _Any, /) -> _Any: ...

    def __lt__(self, other: _Any, /) -> _Any:
        return self._value < other

    @_overload
    def __mod__(self, divisor: _Rational | Self, /) -> Self: ...

    @_overload
    def __mod__(self, divisor: float, /) -> float: ...

    @_overload
    def __mod__(self, divisor: _Any, /) -> _Any: ...

    def __mod__(self, divisor: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            self._value % _to_std_fraction_if_rational(divisor)
        )

    @_overload
    def __mul__(self, other: _Rational | Self, /) -> Self: ...

    @_overload
    def __mul__(self, other: float, /) -> float: ...

    @_overload
    def __mul__(self, other: _Any, /) -> _Any: ...

    def __mul__(self, other: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            self._value * _to_std_fraction_if_rational(other)
        )

    def __neg__(self, /) -> Self:
        return Fraction(-self._value)

    def __pos__(self, /) -> Self:
        return Fraction(+self._value)

    @_overload
    def __pow__(self, exponent: int, /) -> Self: ...

    @_overload
    def __pow__(self, exponent: _Rational | Self, /) -> Self | float: ...

    @_overload
    def __pow__(self, exponent: float, /) -> float: ...

    @_overload
    def __pow__(self, exponent: _Any, /) -> _Any: ...

    def __pow__(self, exponent: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            self._value ** _to_std_fraction_if_rational(exponent)
        )

    @_overload
    def __radd__(self, other: _Rational, /) -> Self: ...

    @_overload
    def __radd__(self, other: float, /) -> float: ...

    @_overload
    def __radd__(self, other: _Any, /) -> _Any: ...

    def __radd__(self, other: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            _to_std_fraction_if_rational(other) + self._value
        )

    @_overload
    def __rdivmod__(self, dividend: _Rational, /) -> tuple[int, Self]: ...

    @_overload
    def __rdivmod__(self, dividend: float, /) -> tuple[float, float]: ...

    @_overload
    def __rdivmod__(self, dividend: _Any, /) -> _Any: ...

    def __rdivmod__(self, dividend: _Any, /) -> _Any:
        result = divmod(_to_std_fraction_if_rational(dividend), self._value)
        return (
            (result[0], Fraction(result[1]))
            if isinstance(result, tuple) and isinstance(result[1], _Fraction)
            else result
        )

    def __repr__(self) -> str:
        return self._value.__repr__()

    @_overload
    def __rfloordiv__(self, dividend: _Rational, /) -> int: ...

    @_overload
    def __rfloordiv__(self, dividend: float, /) -> float: ...

    @_overload
    def __rfloordiv__(self, dividend: _Any, /) -> _Any: ...

    def __rfloordiv__(self, dividend: _Any, /) -> _Any:
        return _to_std_fraction_if_rational(dividend) // self._value

    @_overload
    def __rmod__(self, dividend: _Rational, /) -> Self: ...

    @_overload
    def __rmod__(self, dividend: float, /) -> float: ...

    @_overload
    def __rmod__(self, dividend: _Any, /) -> _Any: ...

    def __rmod__(self, dividend: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(dividend % self._value)

    @_overload
    def __rmul__(self, other: _Rational, /) -> Self: ...

    @_overload
    def __rmul__(self, other: float, /) -> float: ...

    @_overload
    def __rmul__(self, other: _Any, /) -> _Any: ...

    def __rmul__(self, other: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            _to_std_fraction_if_rational(other) * self._value
        )

    @_overload
    def __round__(self, precision: None = ..., /) -> int: ...

    @_overload
    def __round__(self, precision: int, /) -> Self: ...

    def __round__(self, precision: int | None = None, /) -> int | Self:
        return _to_fraction_if_std_fraction(round(self._value, precision))

    @_overload
    def __rpow__(self, base: _Rational, /) -> Self | float: ...

    @_overload
    def __rpow__(self, base: float, /) -> float: ...

    @_overload
    def __rpow__(self, base: _Any, /) -> _Any: ...

    def __rpow__(self, base: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            _to_std_fraction_if_rational(base) ** self._value
        )

    @_overload
    def __rsub__(self, minuend: _Rational, /) -> Self: ...

    @_overload
    def __rsub__(self, minuend: float, /) -> float: ...

    @_overload
    def __rsub__(self, minuend: _Any, /) -> _Any: ...

    def __rsub__(self, minuend: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            _to_std_fraction_if_rational(minuend) - self._value
        )

    @_overload
    def __rtruediv__(self, dividend: _Rational, /) -> Self: ...

    @_overload
    def __rtruediv__(self, dividend: float, /) -> float: ...

    @_overload
    def __rtruediv__(self, dividend: _Any, /) -> _Any: ...

    def __rtruediv__(self, dividend: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            _to_std_fraction_if_rational(dividend) / self._value
        )

    def __str__(self) -> str:
        return self._value.__str__()

    @_overload
    def __sub__(self, subtrahend: _Rational | Self, /) -> Self: ...

    @_overload
    def __sub__(self, subtrahend: float, /) -> float: ...

    @_overload
    def __sub__(self, subtrahend: _Any, /) -> _Any: ...

    def __sub__(self, subtrahend: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            self._value - _to_std_fraction_if_rational(subtrahend)
        )

    @_overload
    def __truediv__(self, divisor: _Rational | Self, /) -> Self: ...

    @_overload
    def __truediv__(self, divisor: float, /) -> float: ...

    @_overload
    def __truediv__(self, divisor: _Any, /) -> _Any: ...

    def __truediv__(self, divisor: _Any, /) -> _Any:
        return _to_fraction_if_std_fraction(
            self._value / _to_std_fraction_if_rational(divisor)
        )

    def __trunc__(self, /) -> int:
        return self._value.__trunc__()


@_overload
def _to_fraction_if_std_fraction(value: _Fraction, /) -> Fraction: ...


@_overload
def _to_fraction_if_std_fraction(value: _Any, /) -> _Any: ...


def _to_fraction_if_std_fraction(value: _Any, /) -> _Any:
    return (
        Fraction(value.numerator, value.denominator)
        if isinstance(value, _Fraction)
        else value
    )


@_overload
def _to_std_fraction_if_rational(value: _numbers.Rational, /) -> _Fraction: ...


@_overload
def _to_std_fraction_if_rational(value: _Any, /) -> _Any: ...


def _to_std_fraction_if_rational(value: _Any, /) -> _Any:
    return (
        _Fraction(int(value.numerator), int(value.denominator))
        if isinstance(value, _numbers.Rational)
        else value
    )
