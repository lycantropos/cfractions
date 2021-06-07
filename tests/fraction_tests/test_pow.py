import math
import sys
from numbers import (Rational,
                     Real)

import pytest
from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         implication,
                         is_fraction_valid,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions, strategies.small_non_negative_integral_rationals)
def test_basic(base: Fraction, exponent: int) -> None:
    result = base ** exponent

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)


@given(strategies.fractions, strategies.small_non_negative_integral_rationals,
       strategies.non_zero_rationals)
def test_ternary_form(base: Fraction, exponent: Rational, modulo: Rational
                      ) -> None:
    result = pow(base, exponent, modulo)

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)
    assert result == (base ** exponent) % modulo


@given(strategies.zero_fractions, strategies.small_positive_integral_fractions)
def test_neutral_element(first: Fraction, second: Fraction) -> None:
    assert first ** second == first


@given(strategies.fractions, strategies.small_non_negative_integral_fractions,
       strategies.small_non_negative_integral_fractions)
def test_exponents_add(base: Fraction,
                       first_exponent: Fraction,
                       second_exponent: Fraction) -> None:
    assert (base ** (first_exponent + second_exponent)
            == (base ** first_exponent) * (base ** second_exponent))


@given(strategies.fractions, strategies.small_non_negative_integral_fractions,
       strategies.small_non_negative_integral_fractions)
def test_exponents_mul(base: Fraction,
                       first_exponent: Fraction,
                       second_exponent: Fraction) -> None:
    assert (base ** (first_exponent * second_exponent)
            == (base ** first_exponent) ** second_exponent)


@given(strategies.fractions, strategies.small_integers)
def test_integer_argument(first: Fraction, second: int) -> None:
    result = first ** second

    assert result == first ** Fraction(second)


@given(strategies.fractions, strategies.rationals,
       strategies.small_non_negative_integral_fractions)
def test_mul_operand(first: Fraction,
                     second: Rational,
                     exponent: Rational) -> None:
    assert ((first * second) ** exponent
            == (first ** exponent) * (second ** exponent))


@given(strategies.integers, strategies.fractions,
       strategies.small_non_negative_integral_fractions)
def test_rmul_operand(first: int,
                      second: Fraction,
                      exponent: Rational) -> None:
    assert ((first * second) ** exponent
            == (first ** exponent) * (second ** exponent))


@given(strategies.fractions, strategies.non_zero_rationals,
       strategies.small_non_negative_integral_fractions)
def test_truediv_operand(first: Fraction,
                         second: Rational,
                         exponent: Rational) -> None:
    assert ((first / second) ** exponent
            == (first ** exponent) / (second ** exponent))


@given(strategies.integers, strategies.non_zero_fractions,
       strategies.small_non_negative_integral_fractions)
def test_rtruediv_operand(first: int,
                          second: Fraction,
                          exponent: Rational) -> None:
    assert ((first / second) ** exponent
            == (first ** exponent) / (second ** exponent))


@given(strategies.int64_fractions,
       strategies.small_non_negative_integral_floats)
def test_float_argument(first: Fraction, second: float) -> None:
    result = first ** second

    assert isinstance(result, float)
    assert implication(math.isfinite(result), math.isfinite(second))
    assert equivalence(math.isnan(result), math.isnan(second))


@skip_reference_counter_test
@given(strategies.fractions, strategies.small_non_negative_integral_fractions)
def test_reference_counter(first: Fraction, second: Fraction) -> None:
    first_refcount_before = sys.getrefcount(first)
    second_refcount_before = sys.getrefcount(second)

    result = first ** second

    first_refcount_after = sys.getrefcount(first)
    second_refcount_after = sys.getrefcount(second)
    assert first_refcount_after == first_refcount_before
    assert second_refcount_after == second_refcount_before


@given(strategies.zero_fractions, strategies.finite_negative_numbers)
def test_zero_base(first: Fraction, second: Real) -> None:
    with pytest.raises(ZeroDivisionError):
        first ** second
