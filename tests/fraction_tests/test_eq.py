from hypothesis import given

from cfractions import Fraction
from tests.utils import (equivalence,
                         implication)
from . import strategies


@given(strategies.fractions)
def test_reflexivity(fraction: Fraction) -> None:
    assert fraction == fraction


@given(strategies.fractions, strategies.fractions)
def test_symmetry(first: Fraction, second: Fraction) -> None:
    assert equivalence(first == second, second == first)


@given(strategies.fractions, strategies.fractions, strategies.fractions)
def test_transitivity(first: Fraction,
                      second: Fraction,
                      third: Fraction) -> None:
    assert implication(first == second and second == third, first == third)


@given(strategies.fractions, strategies.fractions)
def test_connection_with_inequality(first: Fraction, second: Fraction) -> None:
    assert equivalence(not first == second, first != second)
