import sys
from numbers import Complex

from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         is_fraction_valid,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = abs(fraction)

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)


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


@given(strategies.fractions, strategies.finite_numbers)
def test_mul_operand(first: Fraction, second: Complex) -> None:
    assert abs(first * second) == abs(first) * abs(second)


@given(strategies.finite_non_fractions, strategies.fractions)
def test_rmul_operand(first: Complex, second: Fraction) -> None:
    assert abs(first * second) == abs(first) * abs(second)


@given(strategies.fractions, strategies.finite_non_zero_numbers)
def test_truediv_operand(first: Fraction, second: Complex) -> None:
    assert abs(first / second) == abs(first) / abs(second)


@given(strategies.finite_non_fractions, strategies.non_zero_fractions)
def test_rtruediv_operand(first: Complex, second: Fraction) -> None:
    assert abs(first / second) == abs(first) / abs(second)


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    fraction_refcount_before = sys.getrefcount(fraction)

    result = abs(fraction)

    fraction_refcount_after = sys.getrefcount(fraction)
    assert fraction_refcount_after == (fraction_refcount_before
                                       + (result == fraction))
