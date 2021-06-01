from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         implication)
from . import strategies


@given(strategies.fractions)
def test_reflexivity(fraction: Fraction) -> None:
    assert fraction >= fraction


@given(strategies.fractions, strategies.fractions)
def test_antisymmetry(first: Fraction, second: Fraction) -> None:
    assert equivalence(first >= second >= first, first == second)


@given(strategies.fractions, strategies.fractions, strategies.fractions)
def test_transitivity(first: Fraction,
                      second: Fraction,
                      third: Fraction) -> None:
    assert implication(first >= second >= third, first >= third)


@given(strategies.fractions, strategies.fractions)
def test_equivalents(first: Fraction, second: Fraction) -> None:
    result = first >= second

    assert equivalence(result, second <= first)
    assert equivalence(result, first > second or first == second)
    assert equivalence(result, second < first or first == second)
