from hypothesis import given

from cfractions import Fraction

from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = int(fraction)

    assert isinstance(result, int)


@given(strategies.fractions)
def test_value(fraction: Fraction) -> None:
    result = int(fraction)

    assert abs(result - fraction) < 1
