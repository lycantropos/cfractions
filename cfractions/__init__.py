"""Python C API alternative to `fractions` module."""

import typing as _t

__version__ = '2.3.0'

if _t.TYPE_CHECKING:
    from . import _fractions

    Fraction = _fractions.Fraction
else:
    try:
        from . import _cfractions
    except ImportError:
        from . import _fractions

        Fraction = _fractions.Fraction
    else:
        Fraction = _cfractions.Fraction
