import math

from hypothesis import strategies

from cfractions import Fraction

floats = strategies.floats(allow_infinity=True,
                           allow_nan=True)
non_zero_floats = floats.filter(bool)
zero_floats = strategies.sampled_from([-0., 0.])
finite_floats = strategies.floats(allow_infinity=False,
                                  allow_nan=False)
infinite_floats = strategies.sampled_from([math.inf, -math.inf])
nans = strategies.just(math.nan)
integers = numerators = strategies.integers()
finite_non_fraction_numbers = integers | finite_floats
denominators = non_zero_integers = (strategies.integers(max_value=-1)
                                    | strategies.integers(min_value=1))


def is_not_interned(value: int) -> bool:
    return value is not int(str(value))


non_interned_numerators = non_interned_denominators = (
    (strategies.integers(max_value=-256)
     | strategies.integers(min_value=257)).filter(is_not_interned))
zero_integers = strategies.just(0)
zero_fractions = strategies.builds(Fraction)
fractions = (strategies.builds(Fraction, numerators, denominators)
             | strategies.builds(Fraction, finite_floats))
non_zero_fractions = strategies.builds(Fraction, non_zero_integers,
                                       denominators)
zero_numbers = zero_integers | zero_floats | zero_fractions
