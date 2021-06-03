import pickle

from hypothesis import given

from cfractions import Fraction
from . import strategies


@given(strategies.fractions)
def test_round_trip(fraction: Fraction) -> None:
    assert pickle.loads(pickle.dumps(fraction)) == fraction
