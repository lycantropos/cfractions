from numbers import Complex

from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.finite_builtin_non_fractions, strategies.fractions)
def test_connection_with_add(first: Complex, second: Fraction) -> None:
    result = first + second

    assert result == second + first
