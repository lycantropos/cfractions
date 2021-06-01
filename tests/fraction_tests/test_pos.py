import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import skip_reference_counter_test
from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = +fraction

    assert isinstance(result, Fraction)


@given(strategies.fractions)
def test_identity(fraction: Fraction) -> None:
    result = +fraction

    assert result == fraction


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    fraction_refcount_before = sys.getrefcount(fraction)

    result = +fraction

    fraction_refcount_after = sys.getrefcount(fraction)
    assert fraction_refcount_after == (fraction_refcount_before + 1)
