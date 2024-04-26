import math
import numbers as _numbers
import re
from functools import partialmethod
from operator import add
from typing import Any

from hypothesis import strategies as st

from cfractions import Fraction
from tests.utils import Integral, Rational

zero_integers = st.just(0)
small_integers = st.integers(1, 5)
precisions = st.none() | st.integers(-10, 10)
integers = numerators = st.integers()
integers_64 = st.integers(min_value=-(2**63), max_value=2**63 - 1)
negative_integers = st.integers(max_value=-1)
positive_integers = st.integers(min_value=1)
denominators = non_zero_integers = negative_integers | positive_integers
fraction_pattern = re.compile(
    r'\A\s*(?P<sign>[-+]?)(?=\d|\.\d)(?P<num>\d*)'
    r'(?:(?:/(?P<denom>\d+))?'
    r'|(?:\.(?P<decimal>\d*))?'
    r'(?:E(?P<exp>[-+]?\d{1,4}))?)\s*\Z',
    re.IGNORECASE,
)
fractions_strings = st.from_regex(fraction_pattern)
strings = st.text()
like_fraction_strings = (
    fractions_strings | st.builds(add, fractions_strings, strings) | strings
)


def is_not_interned(value: int) -> bool:
    return value is not int(str(value))


non_interned_numerators = non_interned_denominators = (
    st.integers(max_value=-6) | st.integers(min_value=257)
).filter(is_not_interned)
floats = st.floats(allow_infinity=True, allow_nan=True)
finite_negative_floats = st.floats(
    max_value=0, exclude_max=True, allow_infinity=False, allow_nan=False
)
non_zero_floats = floats.filter(bool)
zero_floats = st.sampled_from([-0.0, 0.0])
small_non_negative_integral_floats = small_integers.map(float)
finite_floats = st.floats(allow_infinity=False, allow_nan=False)
finite_non_zero_floats = finite_floats.filter(bool)
infinite_floats = st.sampled_from([math.inf, -math.inf])
nans = st.just(math.nan)
zero_fractions = st.builds(Fraction)
fractions = st.builds(Fraction, numerators, denominators) | st.builds(
    Fraction, finite_floats
)
negative_fractions = st.builds(Fraction, negative_integers, positive_integers)
int64_fractions = st.builds(Fraction, integers_64, denominators)
non_zero_fractions = st.builds(Fraction, non_zero_integers, denominators)
ones: st.SearchStrategy[Rational] = st.just(1)
ones |= st.builds(Fraction, ones)
small_positive_integral_fractions = st.builds(Fraction, small_integers)
finite_non_zero_reals = (
    non_zero_integers | finite_non_zero_floats | non_zero_fractions
)
invalid_components = floats | fractions | fractions_strings
finite_negative_numbers = (
    negative_integers | finite_negative_floats | negative_fractions
)


def call_unwrapped(
    self: Integral, method_name: str, *args: Any, **kwargs: Any
) -> Any:
    return getattr(int(self), method_name)(*args, **kwargs)


@_numbers.Integral.register
class CustomIntegral:
    locals().update(
        {
            method_name: partialmethod(call_unwrapped, method_name)
            for method_name in _numbers.Integral.__abstractmethods__
        }
    )

    @property
    def denominator(self) -> int:
        return 1

    @property
    def numerator(self) -> int:
        return self._value

    def __init__(self, _value: int) -> None:
        self._value = _value

    def __int__(self) -> int:
        return self._value

    def __repr__(self) -> str:
        return f'{type(self).__qualname__}({repr(self._value)})'


@_numbers.Rational.register
class CustomRational:
    def __init__(self, numerator: int, denominator: int) -> None:
        self.denominator, self.numerator = (
            CustomIntegral(denominator),
            CustomIntegral(numerator),
        )

    def __repr__(self) -> str:
        return (
            type(self).__qualname__
            + '('
            + repr(self.numerator)
            + ', '
            + repr(self.denominator)
            + ')'
        )


custom_rationals = st.builds(CustomRational, numerators, denominators)
builtin_rationals = integers | fractions
rationals = builtin_rationals | custom_rationals
finite_builtin_reals = builtin_rationals | finite_floats
finite_non_fractions = finite_builtin_reals | custom_rationals
non_fractions_rationals = integers | custom_rationals
non_fractions = integers | floats | custom_rationals
finite_builtin_non_fractions = integers | finite_floats
non_zero_custom_rationals = st.builds(
    CustomRational, non_zero_integers, denominators
)
finite_non_zero_numbers = finite_non_zero_reals | non_zero_custom_rationals
non_zero_rationals = (
    non_zero_integers | non_zero_fractions | non_zero_custom_rationals
)
zero_custom_rationals = st.builds(CustomRational, zero_integers, denominators)
small_non_negative_integral_fractions = (
    zero_fractions | small_positive_integral_fractions
)
small_non_negative_integral_rationals = (
    zero_integers
    | small_integers
    | small_non_negative_integral_fractions
    | zero_custom_rationals
    | st.builds(CustomRational, small_integers, st.just(1))
)
zero_builtin_reals = zero_integers | zero_floats
zero_rationals = zero_integers | zero_fractions | zero_custom_rationals
zero_non_fractions = zero_builtin_reals | zero_custom_rationals
zero_numbers = zero_builtin_reals | zero_fractions | zero_custom_rationals
numbers = fractions | finite_non_fractions | infinite_floats | nans
