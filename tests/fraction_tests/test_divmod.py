from numbers import (Rational,
                     Real)

import pytest
from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.fractions, strategies.finite_non_zero_numbers)
def test_basic(dividend: Fraction, divisor: Real) -> None:
    result = divmod(dividend, divisor)

    assert isinstance(result, tuple)
    assert len(result) == 2
    assert isinstance(result[0],
                      int if isinstance(divisor, Rational) else float)
    assert isinstance(result[1],
                      Fraction if isinstance(divisor, Rational) else float)


@given(strategies.fractions, strategies.finite_non_zero_numbers)
def test_alternatives(dividend: Fraction, divisor: Real) -> None:
    result = divmod(dividend, divisor)

    assert result == (dividend // divisor, dividend % divisor)


@given(strategies.fractions, strategies.zero_numbers)
def test_zero_divisor(dividend: Fraction, divisor: Real) -> None:
    with pytest.raises(ZeroDivisionError):
        divmod(dividend, divisor)
