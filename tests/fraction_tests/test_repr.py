from hypothesis import given

import cfractions
from cfractions import Fraction
from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = repr(fraction)

    assert result.startswith(Fraction.__qualname__)


@given(strategies.fractions)
def test_round_trip(fraction: Fraction) -> None:
    result = repr(fraction)

    assert eval(result, vars(cfractions)) == fraction
