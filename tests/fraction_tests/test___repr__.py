import sys

from hypothesis import given

import cfractions
from cfractions import Fraction
from tests.utils import skip_reference_counter_test

from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = repr(fraction)

    assert result.startswith(Fraction.__qualname__)


@given(strategies.fractions)
def test_round_trip(fraction: Fraction) -> None:
    result = repr(fraction)

    assert eval(result, vars(cfractions)) == fraction


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    fraction_refcount_before = sys.getrefcount(fraction)

    _result = repr(fraction)

    fraction_refcount_after = sys.getrefcount(fraction)
    assert fraction_refcount_after == fraction_refcount_before
