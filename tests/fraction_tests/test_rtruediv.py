from numbers import (Rational,
                     Real)

import pytest
from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.non_fractions_rationals, strategies.non_zero_fractions)
def test_connection_with_truediv(dividend: Rational,
                                 divisor: Fraction) -> None:
    result = dividend / divisor

    assert result == Fraction(dividend) / divisor


@given(strategies.non_fractions, strategies.zero_fractions)
def test_zero_divisor(dividend: Real, divisor: Fraction) -> None:
    with pytest.raises(ZeroDivisionError):
        dividend / divisor
