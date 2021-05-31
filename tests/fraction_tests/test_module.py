import cfractions


def test_basic() -> None:
    assert cfractions.Fraction.__module__ == cfractions.__name__
