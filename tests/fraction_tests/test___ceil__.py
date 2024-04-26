import math

from hypothesis import given

from cfractions import Fraction

from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = math.ceil(fraction)

    assert isinstance(result, int)


@given(strategies.fractions)
def test_value(fraction: Fraction) -> None:
    result = math.ceil(fraction)

    assert result >= fraction
    assert result < fraction + 1
