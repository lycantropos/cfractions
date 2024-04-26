from __future__ import annotations

from hypothesis import given

from cfractions import Fraction

from . import strategies


@given(strategies.fractions, strategies.precisions)
def test_basic(fraction: Fraction, precision: int | None) -> None:
    result = round(fraction, precision)

    assert isinstance(result, int if precision is None else Fraction)


@given(strategies.fractions, strategies.precisions)
def test_value(fraction: Fraction, precision: int | None) -> None:
    result = round(fraction, precision)

    truncated_fraction = int(fraction)
    base = Fraction(10)
    assert (
        (
            result == truncated_fraction + (-1 if fraction < 0 else 1)
            if (
                abs(truncated_fraction - fraction) == Fraction(1, 2)
                and truncated_fraction % 2 == 1
                or abs(truncated_fraction - fraction) > Fraction(1, 2)
            )
            else result == truncated_fraction
        )
        if precision is None
        else result == (round(fraction * base**precision) / base**precision)
    )
