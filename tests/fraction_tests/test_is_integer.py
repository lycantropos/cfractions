from hypothesis import given

from cfractions import Fraction
from tests.utils import equivalence

from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = fraction.is_integer()

    assert isinstance(result, bool)


@given(strategies.fractions)
def test_alternatives(fraction: Fraction) -> None:
    result = fraction.is_integer()

    assert equivalence(result, fraction.denominator == 1)
