from hypothesis import given

from cfractions.base import Fraction
from . import strategies


@given(strategies.numerators, strategies.denominators)
def test_basic(numerator: int, denominator: int) -> None:
    result = Fraction(numerator, denominator)

    assert bool(numerator) is bool(result.numerator)
    assert (not numerator and not result.numerator
            or not numerator % result.numerator)
    assert (not numerator and not result.numerator
            or not denominator % result.denominator)
