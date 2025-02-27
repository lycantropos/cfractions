import math
import sys

import pytest
from hypothesis import given

from cfractions import Fraction
from tests.utils import Real, equivalence, skip_reference_counter_test

from . import strategies


@given(strategies.fractions, strategies.non_zero_fractions)
def test_basic(dividend: Fraction, divisor: Fraction) -> None:
    result = dividend % divisor

    assert isinstance(result, Fraction)


@given(strategies.fractions, strategies.non_zero_integers)
def test_integer_argument(dividend: Fraction, divisor: int) -> None:
    result = dividend % divisor

    assert result == dividend % Fraction(divisor)


@given(strategies.fractions, strategies.non_zero_floats)
def test_float_argument(dividend: Fraction, divisor: int) -> None:
    result = dividend % divisor

    assert isinstance(result, float)
    assert equivalence(math.isnan(result), math.isnan(divisor))


@given(strategies.fractions, strategies.non_zero_fractions)
def test_value(dividend: Fraction, divisor: Fraction) -> None:
    result = dividend % divisor

    assert (result == 0 and (dividend / divisor == dividend // divisor)) or (
        result > 0
    ) is (divisor > 0)
    assert abs(result) < abs(divisor)


@given(strategies.fractions)
def test_modulo_one(dividend: Fraction) -> None:
    result = dividend % 1

    assert result == dividend - math.floor(dividend)


@given(strategies.fractions, strategies.non_zero_fractions)
def test_connection_with_floordiv(
    dividend: Fraction, divisor: Fraction
) -> None:
    result = dividend % divisor

    assert result + (dividend // divisor) * divisor == dividend


@skip_reference_counter_test
@given(strategies.fractions, strategies.non_zero_fractions)
def test_reference_counter(dividend: Fraction, divisor: Fraction) -> None:
    dividend_refcount_before = sys.getrefcount(dividend)
    divisor_refcount_before = sys.getrefcount(divisor)

    _result = dividend % divisor

    dividend_refcount_after = sys.getrefcount(dividend)
    divisor_refcount_after = sys.getrefcount(divisor)
    assert dividend_refcount_after == dividend_refcount_before
    assert divisor_refcount_after == divisor_refcount_before


@given(strategies.fractions, strategies.zero_numbers)
def test_zero_divisor(dividend: Fraction, divisor: Real) -> None:
    with pytest.raises(ZeroDivisionError):
        dividend % divisor
