import sys

from hypothesis import given

from cfractions import Fraction
from tests.utils import skip_reference_counter_test
from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = str(fraction)

    assert (not fraction and result == str(fraction.numerator)
            or (str(fraction.numerator) in result
                and str(fraction.denominator) in result))


@given(strategies.fractions)
def test_evaluation(fraction: Fraction) -> None:
    result = str(fraction)

    assert eval(result) == float(fraction)


@skip_reference_counter_test
@given(strategies.fractions)
def test_reference_counter(fraction: Fraction) -> None:
    fraction_refcount_before = sys.getrefcount(fraction)

    result = str(fraction)

    fraction_refcount_after = sys.getrefcount(fraction)
    assert fraction_refcount_after == fraction_refcount_before
