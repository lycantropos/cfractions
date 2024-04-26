from hypothesis import given

from cfractions import Fraction
from tests.utils import Real

from . import strategies


@given(strategies.finite_builtin_non_fractions, strategies.fractions)
def test_connection_with_mul(first: Real, second: Fraction) -> None:
    result = first * second

    assert result == second * first
