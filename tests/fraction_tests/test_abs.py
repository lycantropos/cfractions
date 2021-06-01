import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = abs(fraction)

    assert isinstance(result, Fraction)


@given(strategies.fractions)
def test_idempotence(fraction: Fraction) -> None:
    result = abs(fraction)

    assert result == abs(result)


@given(strategies.fractions)
def test_positive_definiteness(fraction: Fraction) -> None:
    result = abs(fraction)

    assert equivalence(not result, not fraction)


@given(strategies.fractions)
def test_evenness(fraction: Fraction) -> None:
    result = abs(fraction)

    assert result == abs(-fraction)


@given(strategies.fractions, strategies.fractions)
def test_multiplicativity(first: Fraction, second: Fraction) -> None:
    result = abs(first * second)

    assert result == abs(first) * abs(second)


@given(strategies.fractions, strategies.fractions)
def test_triangle_inequality(first: Fraction, second: Fraction) -> None:
    result = abs(first + second)

    assert result <= abs(first) + abs(second)


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    value_refcount_before = sys.getrefcount(fraction)

    result = abs(fraction)

    value_refcount_after = sys.getrefcount(fraction)
    assert value_refcount_after == (value_refcount_before
                                    + (fraction == result))
