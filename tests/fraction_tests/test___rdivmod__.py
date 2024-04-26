import numbers

import pytest
from hypothesis import given

from cfractions import Fraction
from tests.utils import Real

from . import strategies


@given(strategies.finite_non_fractions, strategies.non_zero_fractions)
def test_basic(dividend: Real, divisor: Fraction) -> None:
    result = divmod(dividend, divisor)

    assert isinstance(result, tuple)
    assert len(result) == 2
    assert isinstance(
        result[0], int if isinstance(dividend, numbers.Rational) else float
    )
    assert isinstance(
        result[1],
        Fraction if isinstance(dividend, numbers.Rational) else float,
    )


@given(strategies.finite_non_fractions, strategies.non_zero_fractions)
def test_alternatives(dividend: Real, divisor: Fraction) -> None:
    result = divmod(dividend, divisor)

    assert result == (dividend // divisor, dividend % divisor)
    assert not isinstance(dividend, numbers.Rational) or result == divmod(
        Fraction(dividend), divisor
    )


@given(strategies.non_fractions, strategies.zero_fractions)
def test_zero_divisor(dividend: Real, divisor: Fraction) -> None:
    with pytest.raises(ZeroDivisionError):
        divmod(dividend, divisor)
