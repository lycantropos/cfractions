import copy

from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.fractions)
def test_shallow(fraction: Fraction) -> None:
    result = copy.copy(fraction)

    assert result is fraction


@given(strategies.fractions)
def test_deep(fraction: Fraction) -> None:
    result = copy.deepcopy(fraction)

    assert result is fraction
