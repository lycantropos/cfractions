import pytest
from hypothesis import given

from cfractions.base import Fraction
from . import strategies


@given(strategies.numerators, strategies.denominators)
def test_basic(numerator: int, denominator: int) -> None:
    result = Fraction(numerator, denominator)

    assert (not numerator and not result.numerator
            or not numerator % result.numerator)
    assert (not numerator and not result.numerator
            or not denominator % result.denominator)


@given(strategies.finite_floats)
def test_finite_float_argument(value: float) -> None:
    result = Fraction(value)

    numerator, denominator = value.as_integer_ratio()
    assert result.numerator == numerator
    assert result.denominator == denominator


@given(strategies.numerators, strategies.denominators)
def test_properties(numerator: int, denominator: int) -> None:
    result = Fraction(numerator, denominator)

    assert bool(numerator) is bool(result.numerator)
    assert numerator * result.denominator == result.numerator * denominator
    assert result.denominator > 0


@given(strategies.numerators, strategies.zeros)
def test_zero_denominator(numerator: int, denominator: int) -> None:
    with pytest.raises(ZeroDivisionError):
        Fraction(numerator, denominator)
