from numbers import (Complex,
                     Rational)

import pytest
from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.finite_numbers, strategies.non_zero_fractions)
def test_basic(first: Complex, second: Fraction) -> None:
    result = divmod(first, second)

    assert isinstance(result, tuple)
    assert len(result) == 2
    assert isinstance(result[0],
                      int if isinstance(first, Rational) else float)
    assert isinstance(result[1],
                      Fraction if isinstance(first, Rational) else float)


@given(strategies.finite_numbers, strategies.non_zero_fractions)
def test_alternatives(first: Complex, second: Fraction) -> None:
    result = divmod(first, second)

    assert result == (first // second, first % second)
    assert (not isinstance(first, Rational)
            or result == divmod(Fraction(first), second))


@given(strategies.non_fractions, strategies.zero_fractions)
def test_zero_divisor(first: Complex, second: Fraction) -> None:
    with pytest.raises(ZeroDivisionError):
        divmod(first, second)
