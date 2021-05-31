from hypothesis import strategies

from cfractions.base import Fraction

finite_floats = strategies.floats(allow_infinity=False,
                                  allow_nan=False)
numerators = strategies.integers()
denominators = (strategies.integers(max_value=-1)
                | strategies.integers(min_value=1))


def is_not_interned(value: int) -> bool:
    return value is not int(str(value))


non_interned_numerators = numerators.filter(is_not_interned)
non_interned_denominators = denominators.filter(is_not_interned)
zeros = strategies.just(0)
fractions = strategies.builds(Fraction, numerators, denominators)
