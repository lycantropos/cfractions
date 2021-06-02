from numbers import Complex

import pytest
from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.integers, strategies.non_zero_fractions)
def test_connection_with_truediv(first: int, second: Fraction) -> None:
    result = first // second

    assert result == Fraction(first) // second


@given(strategies.non_fraction_numbers, strategies.zero_fractions)
def test_zero_divisor(first: Complex, second: Fraction) -> None:
    with pytest.raises(ZeroDivisionError):
        first // second
