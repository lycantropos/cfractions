import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import equivalence, skip_reference_counter_test

from . import strategies


@given(strategies.fractions)
def test_properties(fraction: Fraction) -> None:
    assert equivalence(bool(fraction), bool(fraction.numerator))


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    fraction_refcount_before = sys.getrefcount(fraction)

    _result = bool(fraction)

    fraction_refcount_after = sys.getrefcount(fraction)
    assert fraction_refcount_after == fraction_refcount_before
