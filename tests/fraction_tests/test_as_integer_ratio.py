from hypothesis import given

from cfractions import Fraction

from . import strategies


@given(strategies.fractions)
def test_basic(fraction: Fraction) -> None:
    result = fraction.as_integer_ratio()

    assert isinstance(result, tuple)
    assert len(result) == 2
    assert all(isinstance(element, int) for element in result)


@given(strategies.fractions)
def test_round_trip(fraction: Fraction) -> None:
    result = fraction.as_integer_ratio()

    assert Fraction(*result).as_integer_ratio() == result
