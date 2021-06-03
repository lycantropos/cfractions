import math

from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = float(fraction)

    assert isinstance(result, float)
    assert not math.isnan(result)
    assert math.isfinite(result)


@given(strategies.fractions)
def test_round_trip(fraction: Fraction) -> None:
    result = float(fraction)

    assert float(Fraction(result)) == result
