from __future__ import annotations

import sys
from typing import Any

import pytest
from hypothesis import given

from cfractions import Fraction
from tests.utils import (
    HasAsIntegerRatioMethod,
    Rational,
    fraction_pattern,
    skip_reference_counter_test,
)

from . import strategies


@given(strategies.numerators, strategies.denominators)
def test_basic(numerator: int, denominator: int) -> None:
    result = Fraction(numerator, denominator)

    assert (
        not numerator and not result.numerator
    ) or not numerator % result.numerator
    assert (
        not numerator and not result.numerator
    ) or not denominator % result.denominator


def test_no_argument() -> None:
    result = Fraction()

    assert result == Fraction(0, 1)


@given(strategies.finite_floats)
def test_finite_float_argument(value: float) -> None:
    result = Fraction(value)

    numerator, denominator = value.as_integer_ratio()
    assert result.numerator == numerator
    assert result.denominator == denominator


@given(strategies.like_fraction_strings)
def test_string_argument(value: str) -> None:
    try:
        result = Fraction(value)
    except ZeroDivisionError:
        assert '/' in value
        assert int(value[value.find('/') + 1 : len(value.rstrip())]) == 0
    except ValueError:
        assert fraction_pattern.fullmatch(value) is None
    else:
        assert isinstance(result, Fraction)


@given(strategies.fractions)
def test_fraction_argument(value: Fraction) -> None:
    result = Fraction(value)

    assert isinstance(result, Fraction)
    assert result == value


@given(strategies.custom_rationals)
def test_custom_rational_argument(value: Rational) -> None:
    result = Fraction(value)

    assert isinstance(result, Fraction)
    assert result == value


@given(strategies.custom_objects_with_valid_as_integer_ratio_method)
def test_custom_object_with_as_integer_ratio_method(
    value: HasAsIntegerRatioMethod[tuple[int, int]],
) -> None:
    result = Fraction(value)

    assert isinstance(result, Fraction)
    assert result.as_integer_ratio() == value.as_integer_ratio()


@given(strategies.custom_objects_with_invalid_return_as_integer_ratio_method)
def test_custom_object_with_invalid_return_as_integer_ratio_method(
    value: HasAsIntegerRatioMethod[Any],
) -> None:
    with pytest.raises(TypeError):
        Fraction(value)


@skip_reference_counter_test
@given(
    strategies.non_interned_numerators, strategies.non_interned_denominators
)
def test_reference_counter(numerator: int, denominator: int) -> None:
    denominator_refcount_before = sys.getrefcount(denominator)
    numerator_refcount_before = sys.getrefcount(numerator)

    result = Fraction(numerator, denominator)

    denominator_refcount_after = sys.getrefcount(denominator)
    numerator_refcount_after = sys.getrefcount(numerator)
    assert denominator_refcount_after == (
        denominator_refcount_before
        + (result.denominator == denominator)
        + (result.numerator == denominator)
    )
    assert numerator_refcount_after == (
        numerator_refcount_before
        + (result.denominator == numerator)
        + (result.numerator == numerator)
    )


@skip_reference_counter_test
@given(strategies.finite_floats)
def test_float_reference_counter(value: int) -> None:
    value_refcount_before = sys.getrefcount(value)

    _result = Fraction(value)

    value_refcount_after = sys.getrefcount(value)
    assert value_refcount_after == value_refcount_before


@skip_reference_counter_test
@given(strategies.custom_objects_with_valid_as_integer_ratio_method)
def test_custom_object_with_as_integer_ratio_method_reference_counter(
    value: HasAsIntegerRatioMethod[Any],
) -> None:
    ratio_refcount_before = sys.getrefcount(value.as_integer_ratio())

    _result = Fraction(value)

    ratio_refcount_after = sys.getrefcount(value.as_integer_ratio())
    assert ratio_refcount_after == ratio_refcount_before


@skip_reference_counter_test
@given(strategies.custom_objects_with_invalid_return_as_integer_ratio_method)
def test_invalid_return_as_integer_ratio_reference_counter(
    value: HasAsIntegerRatioMethod[Any],
) -> None:
    ratio_refcount_before = sys.getrefcount(value.as_integer_ratio())

    with pytest.raises(TypeError):
        Fraction(value)

    ratio_refcount_after = sys.getrefcount(value.as_integer_ratio())
    assert ratio_refcount_after == ratio_refcount_before


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
    with pytest.raises(ValueError):  # ruff:ignore[pytest-raises-too-broad]
        Fraction(value)


@given(strategies.numerators, strategies.invalid_components)
def test_invalid_denominators(
    numerator: int, denominator: Fraction | float | str
) -> None:
    with pytest.raises(TypeError):
        Fraction(numerator, denominator)  # type: ignore


@given(strategies.invalid_components, strategies.denominators)
def test_invalid_numerators(
    numerator: Fraction | float | str, denominator: int
) -> None:
    with pytest.raises(TypeError):
        Fraction(numerator, denominator)  # type: ignore
