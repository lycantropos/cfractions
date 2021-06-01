import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import (implication,
                         skip_reference_counter_test)
from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = hash(fraction)

    assert isinstance(result, int)


@given(strategies.fractions)
def test_determinism(fraction: Fraction) -> None:
    result = hash(fraction)

    assert result == hash(fraction)


@given(strategies.fractions, strategies.fractions)
def test_connection_with_equality(left: Fraction, right: Fraction) -> None:
    assert implication(left == right, hash(left) == hash(right))


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    fraction_refcount_before = sys.getrefcount(fraction)

    result = hash(fraction)

    fraction_refcount_after = sys.getrefcount(fraction)
    assert fraction_refcount_after == fraction_refcount_before
