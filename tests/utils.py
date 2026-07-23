from __future__ import annotations

import fractions
import math
import numbers
import platform
import re
from typing import Protocol, TypeAlias, TypeVar

import pytest

from cfractions import Fraction

_T_co = TypeVar('_T_co', covariant=True)


class HasAsIntegerRatioMethod(Protocol[_T_co]):
    def as_integer_ratio(self, /) -> _T_co: ...


Integral: TypeAlias = int | numbers.Integral
Rational: TypeAlias = (
    Fraction | Integral | fractions.Fraction | numbers.Rational
)
Real: TypeAlias = Rational | float


def equivalence(left_statement: bool, right_statement: bool, /) -> bool:
    return left_statement is right_statement


def implication(antecedent: bool, consequent: bool, /) -> bool:
    return not antecedent or consequent


def is_fraction_valid(fraction: Fraction) -> bool:
    return (
        isinstance(fraction.numerator, int)
        and isinstance(fraction.denominator, int)
        and fraction.denominator > 0
        and math.gcd(fraction.numerator, fraction.denominator) == 1
        and (bool(fraction.numerator) or fraction.denominator == 1)
    )


skip_reference_counter_test = pytest.mark.skipif(
    platform.python_implementation() == 'PyPy',
    reason='PyPy garbage collection is not based on reference counting.',
)
fraction_pattern = re.compile(
    r'\A\s*(?P<sign>[-+]?)(?=\d|\.\d)(?P<num>\d*)'
    r'(?:(?:/(?P<denom>\d+))?'
    r'|(?:\.(?P<decimal>\d*))?'
    r'(?:E(?P<exp>[-+]?\d+))?)\s*\Z',
    re.IGNORECASE,
)
