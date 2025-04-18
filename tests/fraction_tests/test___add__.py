import math
import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import (
    Rational,
    equivalence,
    implication,
    is_fraction_valid,
    skip_reference_counter_test,
)

from . import strategies


@given(strategies.fractions, strategies.rationals)
def test_basic(first: Fraction, second: Rational) -> None:
    result = first + second

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)


@given(strategies.fractions, strategies.integers)
def test_integer_argument(first: Fraction, second: int) -> None:
    result = first + second

    assert result == first + Fraction(second)


@given(strategies.fractions, strategies.fractions)
def test_commutativity(first: Fraction, second: Fraction) -> None:
    assert first + second == second + first


@given(strategies.fractions, strategies.zero_fractions)
def test_neutral_element(first: Fraction, second: Fraction) -> None:
    assert first + second == first == second + first


@given(strategies.fractions, strategies.fractions, strategies.fractions)
def test_associativity(
    first: Fraction, second: Fraction, third: Fraction
) -> None:
    assert (first + second) + third == first + (second + third)


@given(strategies.fractions, strategies.floats)
def test_float_argument(first: Fraction, second: float) -> None:
    result = first + second

    assert isinstance(result, float)
    assert implication(math.isfinite(result), math.isfinite(second))
    assert equivalence(math.isnan(result), math.isnan(second))
    assert implication(
        math.isinf(second) or (not first and not math.isnan(second)),
        result == second,
    )


@skip_reference_counter_test
@given(strategies.fractions, strategies.fractions)
def test_reference_counter(first: Fraction, second: Fraction) -> None:
    first_refcount_before = sys.getrefcount(first)
    second_refcount_before = sys.getrefcount(second)

    _result = first + second

    first_refcount_after = sys.getrefcount(first)
    second_refcount_after = sys.getrefcount(second)
    assert first_refcount_after == first_refcount_before
    assert second_refcount_after == second_refcount_before
