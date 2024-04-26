from hypothesis import given

from cfractions import Fraction
from tests.utils import is_fraction_valid

from . import strategies


@given(strategies.fractions, strategies.positive_integers)
def test_basic(fraction: Fraction, denominator: int) -> None:
    result = fraction.limit_denominator(denominator)

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)


@given(strategies.fractions, strategies.positive_integers)
def test_value(fraction: Fraction, denominator: int) -> None:
    result = fraction.limit_denominator(denominator)

    assert result.denominator <= denominator


@given(strategies.fractions)
def test_default(fraction: Fraction) -> None:
    result = fraction.limit_denominator()

    assert result.denominator <= 10**6
