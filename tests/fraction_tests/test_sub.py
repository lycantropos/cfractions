import math
import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         implication,
                         is_fraction_valid,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions, strategies.fractions)
def test_basic(first: Fraction, second: Fraction) -> None:
    result = first - second

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)


@given(strategies.fractions)
def test_diagonal(fraction: Fraction) -> None:
    assert not fraction - fraction


@given(strategies.fractions, strategies.fractions)
def test_commutative_case(first: Fraction, second: Fraction) -> None:
    assert equivalence(first - second == second - first, first == second)


@given(strategies.fractions, strategies.zero_fractions)
def test_right_neutral_element(first: Fraction, second: Fraction) -> None:
    assert first - second == first


@given(strategies.fractions, strategies.integers)
def test_integer_argument(first: Fraction, second: int) -> None:
    result = first - second

    assert result == first - Fraction(second)


@given(strategies.fractions, strategies.floats)
def test_float_argument(first: Fraction, second: float) -> None:
    result = first - second

    assert isinstance(result, float)
    assert equivalence(math.isfinite(result), math.isfinite(second))
    assert equivalence(math.isnan(result), math.isnan(second))
    assert implication(math.isinf(second)
                       or not first and not math.isnan(second),
                       result == -second)


@given(strategies.fractions, strategies.fractions)
def test_alternatives(first: Fraction, second: Fraction) -> None:
    result = first - second

    assert result == first + (-second)
    assert result == -((-first) + second)


@skip_reference_counter_test
@given(strategies.fractions, strategies.fractions)
def test_reference_counter(first: Fraction, second: Fraction) -> None:
    first_refcount_before = sys.getrefcount(first)
    second_refcount_before = sys.getrefcount(second)

    result = first - second

    first_refcount_after = sys.getrefcount(first)
    second_refcount_after = sys.getrefcount(second)
    assert first_refcount_after == first_refcount_before
    assert second_refcount_after == second_refcount_before
