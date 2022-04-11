import sys
from numbers import Real

from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         is_fraction_valid,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = -fraction

    assert isinstance(result, Fraction)
    assert is_fraction_valid(result)


@given(strategies.fractions)
def test_fixed_point(fraction: Fraction) -> None:
    result = -fraction

    assert equivalence(fraction == result, not fraction)


@given(strategies.fractions)
def test_involution(fraction: Fraction) -> None:
    result = -fraction

    assert fraction == -result


@given(strategies.fractions, strategies.finite_builtin_reals)
def test_add_operand(first: Fraction, second: Real) -> None:
    assert -(first + second) == (-first) + (-second)


@given(strategies.finite_builtin_non_fractions, strategies.fractions)
def test_radd_operand(first: Real, second: Fraction) -> None:
    assert -(first + second) == (-first) + (-second)


@given(strategies.fractions, strategies.finite_builtin_reals)
def test_sub_operand(first: Fraction, second: Real) -> None:
    assert -(first - second) == (-first) - (-second)


@given(strategies.finite_builtin_non_fractions, strategies.fractions)
def test_rsub_operand(first: Real, second: Fraction) -> None:
    assert -(first - second) == (-first) - (-second)


@given(strategies.fractions, strategies.finite_builtin_reals)
def test_mul_operand(first: Fraction, second: Real) -> None:
    assert -(first * second) == (-first) * second == first * (-second)


@given(strategies.finite_builtin_non_fractions, strategies.fractions)
def test_rmul_operand(first: Real, second: Fraction) -> None:
    assert -(first * second) == (-first) * second == first * (-second)


@given(strategies.fractions, strategies.finite_non_zero_reals)
def test_truediv_operand(first: Fraction, second: Real) -> None:
    assert -(first / second) == (-first) / second == first / (-second)


@given(strategies.finite_builtin_non_fractions, strategies.non_zero_fractions)
def test_rtruediv_operand(first: Real, second: Fraction) -> None:
    assert -(first / second) == (-first) / second == first / (-second)


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    fraction_refcount_before = sys.getrefcount(fraction)

    result = -fraction

    fraction_refcount_after = sys.getrefcount(fraction)
    assert fraction_refcount_after == fraction_refcount_before
