import sys

import pytest
from hypothesis import given

from cfractions import Fraction
from tests.utils import skip_reference_counter_test
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


@skip_reference_counter_test
@given(strategies.non_interned_numerators,
       strategies.non_interned_denominators)
def test_reference_counter(numerator: int, denominator: int) -> None:
    denominator_refcount_before = sys.getrefcount(denominator)
    numerator_refcount_before = sys.getrefcount(numerator)

    result = Fraction(numerator, denominator)

    denominator_refcount_after = sys.getrefcount(denominator)
    numerator_refcount_after = sys.getrefcount(numerator)
    assert (denominator_refcount_after
            == (denominator_refcount_before
                + (result.denominator == denominator)
                + (result.numerator == denominator)))
    assert numerator_refcount_after == (numerator_refcount_before
                                        + (result.denominator == numerator)
                                        + (result.numerator == numerator))


@skip_reference_counter_test
@given(strategies.finite_floats)
def test_float_reference_counter(value: int) -> None:
    value_refcount_before = sys.getrefcount(value)

    result = Fraction(value)

    value_refcount_after = sys.getrefcount(value)
    assert value_refcount_after == value_refcount_before


@given(strategies.numerators, strategies.denominators)
def test_properties(numerator: int, denominator: int) -> None:
    result = Fraction(numerator, denominator)

    assert bool(numerator) is bool(result.numerator)
    assert numerator * result.denominator == result.numerator * denominator
    assert result.denominator > 0


@given(strategies.numerators, strategies.zero_integers)
def test_zero_denominator(numerator: int, denominator: int) -> None:
    with pytest.raises(ZeroDivisionError):
        Fraction(numerator, denominator)


@given(strategies.infinite_floats)
def test_infinite_float_argument(value: float) -> None:
    with pytest.raises(OverflowError):
        Fraction(value)


@given(strategies.nans)
def test_nan_float_argument(value: float) -> None:
    with pytest.raises(ValueError):
        Fraction(value)
