from numbers import (Complex,
                     Rational)

from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.fractions, strategies.finite_non_zero_numbers)
def test_basic(first: Fraction, second: Complex) -> None:
    result = divmod(first, second)

    assert isinstance(result, tuple)
    assert len(result) == 2
    assert isinstance(result[0],
                      int if isinstance(second, Rational) else float)
    assert isinstance(result[1],
                      Fraction if isinstance(second, Rational) else float)


@given(strategies.fractions, strategies.finite_non_zero_numbers)
def test_alternatives(first: Fraction, second: Complex) -> None:
    result = divmod(first, second)

    assert result == (first // second, first % second)
