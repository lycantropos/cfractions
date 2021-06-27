from typing import Optional

from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.fractions, strategies.precisions)
def test_basic(fraction: Fraction, precision: Optional[int]) -> None:
    result = round(fraction, precision)

    assert isinstance(result, int if precision is None else Fraction)


@given(strategies.fractions, strategies.precisions)
def test_value(fraction: Fraction, precision: Optional[int]) -> None:
    result = round(fraction, precision)

    base = Fraction(10)
    assert ((result >= int(fraction)
             if fraction > 0
             else result <= int(fraction))
            if precision is None
            else result == (round(fraction * base ** precision)
                            / base ** precision))
