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
def test_basic(base: Fraction, exponent: Rational) -> None:
    result = base ** exponent

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)


@given(strategies.zero_fractions, strategies.small_positive_integral_fractions)
def test_left_absorbing_element(base: Fraction, exponent: Fraction) -> None:
    assert base ** exponent == base


@given(strategies.fractions, strategies.ones)
def test_right_neutral_element(base: Fraction, exponent: Rational) -> None:
    assert base ** exponent == base


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
def test_integer_argument(base: Fraction, exponent: int) -> None:
    result = base ** exponent

    assert result == base ** Fraction(exponent)


@given(strategies.fractions, strategies.rationals,
       strategies.small_non_negative_integral_fractions)
def test_mul_operand(first_base: Fraction,
                     second_base: Rational,
                     exponent: Rational) -> None:
    assert ((first_base * second_base) ** exponent
            == (first_base ** exponent) * (second_base ** exponent))


@given(strategies.integers, strategies.fractions,
       strategies.small_non_negative_integral_fractions)
def test_rmul_operand(first_base: int,
                      second_base: Fraction,
                      exponent: Rational) -> None:
    assert ((first_base * second_base) ** exponent
            == (first_base ** exponent) * (second_base ** exponent))


@given(strategies.fractions, strategies.non_zero_rationals,
       strategies.small_non_negative_integral_fractions)
def test_truediv_operand(first_base: Fraction,
                         second_base: Rational,
                         exponent: Rational) -> None:
    assert ((first_base / second_base) ** exponent
            == (first_base ** exponent) / (second_base ** exponent))


@given(strategies.integers, strategies.non_zero_fractions,
       strategies.small_non_negative_integral_fractions)
def test_rtruediv_operand(first_base: int,
                          second_base: Fraction,
                          exponent: Fraction) -> None:
    assert ((first_base / second_base) ** exponent
            == (first_base ** exponent) / (second_base ** exponent))


@given(strategies.int64_fractions,
       strategies.small_non_negative_integral_floats)
def test_float_argument(base: Fraction, exponent: float) -> None:
    result = base ** exponent

    assert isinstance(result, float)
    assert implication(math.isfinite(result), math.isfinite(exponent))
    assert equivalence(math.isnan(result), math.isnan(exponent))


@skip_reference_counter_test
@given(strategies.fractions, strategies.small_non_negative_integral_fractions)
def test_reference_counter(base: Fraction, exponent: Fraction) -> None:
    first_refcount_before = sys.getrefcount(base)
    second_refcount_before = sys.getrefcount(exponent)

    result = base ** exponent

    first_refcount_after = sys.getrefcount(base)
    second_refcount_after = sys.getrefcount(exponent)
    assert first_refcount_after == first_refcount_before
    assert second_refcount_after == second_refcount_before


@given(strategies.zero_fractions, strategies.finite_negative_numbers)
def test_zero_base(base: Fraction, exponent: Real) -> None:
    with pytest.raises(ZeroDivisionError):
        base ** exponent
