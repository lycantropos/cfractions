import math
import sys
from numbers import Complex

import pytest
from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions, strategies.non_zero_fractions)
def test_basic(first: Fraction, second: Fraction) -> None:
    result = first % second

    assert isinstance(result, Fraction)


@given(strategies.fractions, strategies.non_zero_integers)
def test_integer_argument(first: Fraction, second: int) -> None:
    result = first % second

    assert result == first % Fraction(second)


@given(strategies.fractions, strategies.non_zero_floats)
def test_float_argument(first: Fraction, second: int) -> None:
    result = first % second

    assert isinstance(result, float)
    assert equivalence(math.isnan(result), math.isnan(second))


@given(strategies.fractions, strategies.non_zero_fractions)
def test_value(first: Fraction, second: Fraction) -> None:
    result = first % second

    assert (result == 0 and (first / second == first // second)
            or (result > 0) is (second > 0))
    assert abs(result) < abs(second)


@given(strategies.fractions)
def test_modulo_one(first: Fraction) -> None:
    result = first % 1

    assert result == first - math.floor(first)


@given(strategies.fractions, strategies.non_zero_fractions)
def test_connection_with_floordiv(first: Fraction, second: Fraction) -> None:
    result = first % second

    assert result + (first // second) * second == first


@skip_reference_counter_test
@given(strategies.fractions, strategies.non_zero_fractions)
def test_reference_counter(first: Fraction, second: Fraction) -> None:
    first_refcount_before = sys.getrefcount(first)
    second_refcount_before = sys.getrefcount(second)

    result = first % second

    first_refcount_after = sys.getrefcount(first)
    second_refcount_after = sys.getrefcount(second)
    assert first_refcount_after == first_refcount_before
    assert second_refcount_after == second_refcount_before


@given(strategies.fractions, strategies.zero_numbers)
def test_zero_divisor(first: Fraction, second: Complex) -> None:
    with pytest.raises(ZeroDivisionError):
        first % second
