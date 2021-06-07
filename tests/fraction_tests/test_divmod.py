from numbers import (Rational,
                     Real)

import pytest
from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.fractions, strategies.finite_non_zero_numbers)
def test_basic(first: Fraction, second: Real) -> None:
    result = divmod(first, second)

    assert isinstance(result, tuple)
    assert len(result) == 2
    assert isinstance(result[0],
                      int if isinstance(second, Rational) else float)
    assert isinstance(result[1],
                      Fraction if isinstance(second, Rational) else float)


@given(strategies.fractions, strategies.finite_non_zero_numbers)
def test_alternatives(first: Fraction, second: Real) -> None:
    result = divmod(first, second)

    assert result == (first // second, first % second)


@given(strategies.fractions, strategies.zero_numbers)
def test_zero_divisor(first: Fraction, second: Real) -> None:
    with pytest.raises(ZeroDivisionError):
        divmod(first, second)
