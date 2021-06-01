import math
import platform

import pytest

from cfractions import Fraction


def equivalence(left_statement: bool, right_statement: bool) -> bool:
    return left_statement is right_statement


def implication(antecedent: bool, consequent: bool) -> bool:
    return not antecedent or consequent


def is_fraction_valid(fraction: Fraction) -> bool:
    return (fraction.denominator > 0
            and math.gcd(fraction.numerator, fraction.denominator) == 1
            and (bool(fraction.numerator) or fraction.denominator == 1))


skip_reference_counter_test = pytest.mark.skipif(
        platform.python_implementation() == 'PyPy',
        reason='PyPy\'s garbage collection '
               'is not based on reference counting.')
