import math
import sys
from numbers import Real

import pytest
from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         implication,
                         is_fraction_valid,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions, strategies.non_zero_fractions)
def test_basic(first: Fraction, second: Fraction) -> None:
    result = first / second

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)


@given(strategies.non_zero_fractions, strategies.non_zero_fractions)
def test_commutative_case(first: Fraction, second: Fraction) -> None:
    assert equivalence(first / second == second / first,
                       abs(first) == abs(second))


@given(strategies.zero_fractions, strategies.non_zero_fractions)
def test_left_absorbing_element(first: Fraction, second: Fraction) -> None:
    assert first / second == first


@given(strategies.fractions, strategies.non_zero_integers)
def test_integer_argument(first: Fraction, second: int) -> None:
    result = first / second

    assert result == first / Fraction(second)


@given(strategies.fractions, strategies.non_zero_floats)
def test_float_argument(first: Fraction, second: float) -> None:
    result = first / second

    assert isinstance(result, float)
    assert equivalence(math.isnan(result), math.isnan(second))
    assert implication(math.isinf(second)
                       or not first and not math.isnan(second), not result)


@skip_reference_counter_test
@given(strategies.fractions, strategies.non_zero_fractions)
def test_reference_counter(first: Fraction, second: Fraction) -> None:
    first_refcount_before = sys.getrefcount(first)
    second_refcount_before = sys.getrefcount(second)

    result = first / second

    first_refcount_after = sys.getrefcount(first)
    second_refcount_after = sys.getrefcount(second)
    assert first_refcount_after == first_refcount_before
    assert second_refcount_after == second_refcount_before


@given(strategies.fractions, strategies.zero_numbers)
def test_zero_divisor(first: Fraction, second: Real) -> None:
    with pytest.raises(ZeroDivisionError):
        first / second
