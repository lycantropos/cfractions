from numbers import Complex

import pytest
from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.non_fractions_rationals,
       strategies.small_non_negative_integral_fractions)
def test_connection_with_truediv(first: int, second: Fraction) -> None:
    result = first ** second

    assert result == Fraction(first) ** second


@given(strategies.zero_non_fractions, strategies.negative_fractions)
def test_zero_base(first: Complex, second: Fraction) -> None:
    with pytest.raises(ZeroDivisionError):
        first ** second
