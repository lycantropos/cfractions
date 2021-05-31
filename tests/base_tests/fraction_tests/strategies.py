from hypothesis import strategies

from cfractions.base import Fraction

finite_floats = strategies.floats(allow_infinity=False,
                                  allow_nan=False)
numerators = strategies.integers()
denominators = (strategies.integers(max_value=-1)
                | strategies.integers(min_value=1))
zeros = strategies.just(0)
fractions = strategies.builds(Fraction, numerators, denominators)
