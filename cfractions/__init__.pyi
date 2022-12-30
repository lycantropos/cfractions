import numbers as _numbers
from typing import (Any as _Any,
                    Dict as _Dict,
                    Optional as _Optional,
                    Tuple as _Tuple,
                    Union as _Union,
                    overload as _overload)

__version__: str = ...


class Fraction(_numbers.Rational):
    @property
    def numerator(self) -> int:
        ...

    @property
    def denominator(self) -> int:
        ...

    def as_integer_ratio(self) -> _Tuple[int, int]:
        ...

    def limit_denominator(self, max_denominator: int = ...) -> 'Fraction':
        ...

    @_overload
    def __new__(cls,
                value: _Union[_numbers.Rational, float, str] = ...,
                _: None = ...) -> 'Fraction':
        ...

    @_overload
    def __new__(cls,
                numerator: int,
                denominator: int) -> 'Fraction':
        ...

    def __abs__(self) -> 'Fraction':
        ...

    @_overload
    def __add__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __add__(self, other: float) -> float:
        ...

    def __ceil__(self) -> int:
        ...

    def __copy__(self) -> 'Fraction':
        ...

    def __deepcopy__(self,
                     memo: _Optional[_Dict[int, _Any]] = ...) -> 'Fraction':
        ...

    @_overload
    def __divmod__(self, other: _numbers.Rational) -> _Tuple[int, 'Fraction']:
        ...

    @_overload
    def __divmod__(self, other: float) -> _Tuple[float, float]:
        ...

    @_overload
    def __eq__(self, other: _numbers.Rational) -> bool:
        ...

    @_overload
    def __eq__(self, other: float) -> bool:
        ...

    @_overload
    def __eq__(self, other: _Any) -> _Any:
        ...

    def __floor__(self) -> int:
        ...

    @_overload
    def __floordiv__(self, other: _numbers.Rational) -> int:
        ...

    @_overload
    def __floordiv__(self, other: float) -> float:
        ...

    @_overload
    def __ge__(self, other: _numbers.Rational) -> bool:
        ...

    @_overload
    def __ge__(self, other: float) -> bool:
        ...

    @_overload
    def __ge__(self, other: _Any) -> _Any:
        ...

    @_overload
    def __gt__(self, other: _numbers.Rational) -> bool:
        ...

    @_overload
    def __gt__(self, other: float) -> bool:
        ...

    @_overload
    def __gt__(self, other: _Any) -> _Any:
        ...

    def __hash__(self) -> int:
        ...

    @_overload
    def __le__(self, other: _numbers.Rational) -> bool:
        ...

    @_overload
    def __le__(self, other: float) -> bool:
        ...

    @_overload
    def __le__(self, other: _Any) -> _Any:
        ...

    @_overload
    def __lt__(self, other: _numbers.Rational) -> bool:
        ...

    @_overload
    def __lt__(self, other: float) -> bool:
        ...

    @_overload
    def __lt__(self, other: _Any) -> _Any:
        ...

    @_overload
    def __mod__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __mod__(self, other: float) -> float:
        ...

    @_overload
    def __mul__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __mul__(self, other: float) -> float:
        ...

    def __neg__(self) -> 'Fraction':
        ...

    def __pos__(self) -> 'Fraction':
        ...

    @_overload
    def __pow__(self, exponent: int) -> 'Fraction':
        ...

    @_overload
    def __pow__(self, exponent: _numbers.Rational
                ) -> _Union['Fraction', float]:
        ...

    @_overload
    def __pow__(self, exponent: float) -> float:
        ...

    @_overload
    def __radd__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __radd__(self, other: float) -> float:
        ...

    @_overload
    def __rdivmod__(self, other: _numbers.Rational) -> _Tuple[int, 'Fraction']:
        ...

    @_overload
    def __rdivmod__(self, other: float) -> _Tuple[float, float]:
        ...

    @_overload
    def __rfloordiv__(self, other: _numbers.Rational) -> int:
        ...

    @_overload
    def __rfloordiv__(self, other: float) -> float:
        ...

    @_overload
    def __rmod__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __rmod__(self, other: float) -> float:
        ...

    @_overload
    def __rmul__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __rmul__(self, other: float) -> float:
        ...

    @_overload
    def __round__(self, precision: None = ...) -> int:
        ...

    @_overload
    def __round__(self, precision: int) -> 'Fraction':
        ...

    @_overload
    def __rpow__(self, base: _numbers.Rational) -> _Union['Fraction', float]:
        ...

    @_overload
    def __rpow__(self, base: float) -> float:
        ...

    @_overload
    def __rsub__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __rsub__(self, other: float) -> float:
        ...

    @_overload
    def __rtruediv__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __rtruediv__(self, other: float) -> float:
        ...

    @_overload
    def __sub__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __sub__(self, other: float) -> float:
        ...

    @_overload
    def __truediv__(self, other: _numbers.Rational) -> 'Fraction':
        ...

    @_overload
    def __truediv__(self, other: float) -> float:
        ...

    def __trunc__(self) -> int:
        ...
