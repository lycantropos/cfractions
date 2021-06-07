import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         implication,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions)
def test_reflexivity(fraction: Fraction) -> None:
    assert fraction <= fraction


@given(strategies.fractions, strategies.fractions)
def test_antisymmetry(first: Fraction, second: Fraction) -> None:
    assert equivalence(first <= second <= first, first == second)


@given(strategies.fractions, strategies.fractions, strategies.fractions)
def test_transitivity(first: Fraction,
                      second: Fraction,
                      third: Fraction) -> None:
    assert implication(first <= second <= third, first <= third)


@given(strategies.fractions, strategies.fractions)
def test_equivalents(first: Fraction, second: Fraction) -> None:
    result = first <= second

    assert equivalence(result, second >= first)
    assert equivalence(result, first < second or first == second)
    assert equivalence(result, second > first or first == second)


@given(strategies.fractions, strategies.floats)
def test_float_operand(first: Fraction, second: float) -> None:
    assert equivalence(first <= second, float(first) <= second)


@skip_reference_counter_test
@given(strategies.fractions, strategies.fractions)
def test_reference_counter(first: Fraction, second: Fraction) -> None:
    first_refcount_before = sys.getrefcount(first)
    second_refcount_before = sys.getrefcount(second)

    result = first <= second

    first_refcount_after = sys.getrefcount(first)
    second_refcount_after = sys.getrefcount(second)
    assert first_refcount_after == first_refcount_before
    assert second_refcount_after == second_refcount_before
