import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import skip_reference_counter_test
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
def test_non_negativeness(fraction: Fraction) -> None:
    result = abs(fraction)

    assert result >= 0


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    value_refcount_before = sys.getrefcount(fraction)

    result = abs(fraction)

    value_refcount_after = sys.getrefcount(fraction)
    assert value_refcount_after == (value_refcount_before
                                    + (fraction == result))
