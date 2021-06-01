#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include <structmember.h>

#define PY39_OR_MORE PY_VERSION_HEX >= 0x03090000

typedef struct {
  PyObject_HEAD PyObject *numerator;
  PyObject *denominator;
} FractionObject;

static void Fraction_dealloc(FractionObject *self) {
  Py_XDECREF(self->numerator);
  Py_XDECREF(self->denominator);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *Fraction_new(PyTypeObject *cls, PyObject *args,
                              PyObject *kwargs) {
  FractionObject *self;
  self = (FractionObject *)(cls->tp_alloc(cls, 0));
  if (self) {
    self->numerator = PyLong_FromLong(0);
    if (!self->numerator) {
      Py_DECREF(self);
      return NULL;
    }
    self->denominator = PyLong_FromLong(1);
    if (!self->denominator) {
      Py_DECREF(self);
      return NULL;
    }
  }
  return (PyObject *)self;
}

static PyObject *Fractions_richcompare(FractionObject *self,
                                       FractionObject *other, int op) {
  if (op == Py_EQ) {
    return PyBool_FromLong(
        PyObject_RichCompareBool(self->numerator, other->numerator, op) &&
        PyObject_RichCompareBool(self->denominator, other->denominator, op));
  } else if (op == Py_NE) {
    return PyBool_FromLong(
        PyObject_RichCompareBool(self->numerator, other->numerator, op) ||
        PyObject_RichCompareBool(self->denominator, other->denominator, op));
  } else {
    PyObject *result, *left, *right;
    left = PyNumber_Multiply(self->numerator, other->denominator);
    if (!left) return NULL;
    right = PyNumber_Multiply(other->numerator, self->denominator);
    if (!right) {
      Py_DECREF(left);
      return NULL;
    }
    result = PyObject_RichCompare(left, right, op);
    Py_DECREF(left);
    Py_DECREF(right);
    return result;
  }
}

static PyTypeObject FractionType;

static PyObject *Fraction_richcompare(FractionObject *self, PyObject *other,
                                      int op) {
  if (PyObject_TypeCheck(other, &FractionType)) {
    return Fractions_richcompare(self, (FractionObject *)other, op);
  } else if (PyLong_Check(other)) {
    PyObject *result, *tmp;
    if (op == Py_EQ) {
      tmp = PyLong_FromLong(1);
      if (!tmp) return NULL;
      result = PyBool_FromLong(
          PyObject_RichCompareBool(self->denominator, tmp, op) &&
          PyObject_RichCompareBool(self->numerator, other, op));
      Py_DECREF(tmp);
      return result;
    } else if (op == Py_NE) {
      tmp = PyLong_FromLong(1);
      if (!tmp) return NULL;
      result = PyBool_FromLong(
          PyObject_RichCompareBool(self->denominator, tmp, op) ||
          PyObject_RichCompareBool(self->numerator, other, op));
      Py_DECREF(tmp);
      return result;
    } else {
      tmp = PyNumber_Multiply(other, self->denominator);
      result = PyObject_RichCompare(self->numerator, tmp, op);
      Py_DECREF(tmp);
      return result;
    }
  } else if (PyFloat_Check(other)) {
    PyObject *other_fraction, *result;
    if (!isfinite(PyFloat_AS_DOUBLE(other))) Py_RETURN_FALSE;
#if PY39_OR_MORE
    other_fraction = PyObject_CallOneArg((PyObject *)&FractionType, other);
#else
    other_fraction =
        PyObject_CallFunctionObjArgs((PyObject *)&FractionType, other, NULL);
#endif
    if (!other_fraction) return NULL;
    result = Fractions_richcompare(self, (FractionObject *)other_fraction, op);
    Py_DECREF(other_fraction);
    return result;
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static int Fraction_init(FractionObject *self, PyObject *args) {
  PyObject *numerator = NULL, *denominator = NULL, *tmp, *gcd;
  if (!PyArg_ParseTuple(args, "|OO", &numerator, &denominator)) return -1;
  if (denominator) {
    if (!PyLong_Check(numerator)) {
      PyErr_SetString(PyExc_TypeError,
                      "Numerator should be an integer "
                      "when denominator is specified.");
      return -1;
    }
    if (!PyLong_Check(denominator)) {
      PyErr_SetString(PyExc_TypeError, "Denominator should be an integer.");
      return -1;
    }
    if (!PyObject_IsTrue(denominator)) {
      PyErr_SetString(PyExc_ZeroDivisionError,
                      "Denominator should be non-zero.");
      return -1;
    }

    tmp = PyLong_FromLong(0);
    if (PyObject_RichCompareBool(denominator, tmp, Py_LT)) {
      Py_DECREF(tmp);
      numerator = PyNumber_Negative(numerator);
      if (!numerator) return -1;
      denominator = PyNumber_Negative(denominator);
      if (!denominator) {
        Py_DECREF(numerator);
        return -1;
      }
    } else {
      Py_DECREF(tmp);
      Py_INCREF(numerator);
      Py_INCREF(denominator);
    }

    gcd = _PyLong_GCD(numerator, denominator);
    if (!gcd) {
      Py_DECREF(denominator);
      Py_DECREF(numerator);
      return -1;
    }

    tmp = PyLong_FromLong(1);
    if (PyObject_RichCompareBool(gcd, tmp, Py_NE)) {
      Py_DECREF(tmp);
      tmp = numerator;
      numerator = PyNumber_FloorDivide(numerator, gcd);
      Py_DECREF(tmp);
      if (!numerator) {
        Py_DECREF(denominator);
        Py_DECREF(gcd);
        return -1;
      }
      tmp = denominator;
      denominator = PyNumber_FloorDivide(denominator, gcd);
      Py_DECREF(tmp);
      if (!denominator) {
        Py_DECREF(numerator);
        Py_DECREF(gcd);
        return -1;
      }
    } else {
      Py_DECREF(tmp);
    }
    Py_DECREF(gcd);

    tmp = self->numerator;
    self->numerator = numerator;
    Py_XDECREF(tmp);

    tmp = self->denominator;
    self->denominator = denominator;
    Py_XDECREF(tmp);
  } else if (numerator) {
    if (PyLong_Check(numerator))
      Py_INCREF(numerator);
    else if (PyFloat_Check(numerator)) {
      double value = PyFloat_AS_DOUBLE(numerator);
      int exponent;
      PyObject *exponent_object;
      size_t index;
      if (isinf(value)) {
        PyErr_SetString(PyExc_OverflowError,
                        "Cannot construct Fraction from infinity.");
        return -1;
      }
      if (isnan(value)) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot construct Fraction from NaN.");
        return -1;
      }
      value = frexp(value, &exponent);
      for (index = 0; index < 300 && value != floor(value); ++index) {
        value *= 2.0;
        exponent--;
      }
      numerator = PyLong_FromDouble(value);
      if (!numerator) return -1;
      denominator = PyLong_FromLong(1);
      if (!denominator) {
        Py_DECREF(numerator);
        return -1;
      }
      exponent_object = PyLong_FromLong(abs(exponent));
      if (!exponent_object) {
        Py_DECREF(numerator);
        Py_DECREF(denominator);
        return -1;
      }
      if (exponent > 0) {
        tmp = numerator;
        numerator = PyNumber_Lshift(numerator, exponent_object);
        Py_DECREF(tmp);
        if (!numerator) {
          Py_DECREF(denominator);
          Py_DECREF(exponent_object);
          return -1;
        }
      } else {
        tmp = denominator;
        denominator = PyNumber_Lshift(denominator, exponent_object);
        Py_DECREF(tmp);
        if (!denominator) {
          Py_DECREF(numerator);
          Py_DECREF(exponent_object);
          return -1;
        }
      }
      Py_DECREF(exponent_object);

      tmp = self->denominator;
      self->denominator = denominator;
      Py_XDECREF(tmp);
    } else {
      PyErr_SetString(PyExc_TypeError,
                      "Numerator should be either an integer "
                      "or a floating point number "
                      "when denominator is not specified.");
      return -1;
    }
    tmp = self->numerator;
    self->numerator = numerator;
    Py_XDECREF(tmp);
  }
  return 0;
}

static PyMemberDef Fraction_members[] = {
    {"numerator", T_OBJECT_EX, offsetof(FractionObject, numerator), READONLY,
     "Numerator of the fraction."},
    {"denominator", T_OBJECT_EX, offsetof(FractionObject, denominator),
     READONLY, "Denominator of the fraction."},
    {NULL} /* sentinel */
};

static FractionObject *Fraction_negative(FractionObject *self) {
  PyObject *numerator_negative;
  FractionObject *result;
  numerator_negative = PyNumber_Negative(self->numerator);
  if (!numerator_negative) return NULL;
  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(numerator_negative);
    return NULL;
  }
  result->numerator = numerator_negative;
  Py_INCREF(self->denominator);
  result->denominator = self->denominator;
  return result;
}

static FractionObject *Fraction_abs(FractionObject *self) {
  FractionObject *result;
  PyObject *tmp = PyLong_FromLong(0);
  if (PyObject_RichCompareBool(self->numerator, tmp, Py_LT))
    result = Fraction_negative(self);
  else {
    Py_INCREF((PyObject *)self);
    result = self;
  }
  Py_DECREF(tmp);
  return result;
}

static PyObject *Fraction_float(FractionObject *self) {
  return PyNumber_TrueDivide(self->numerator, self->denominator);
}

static FractionObject *Fractions_mul(FractionObject *self,
                                     FractionObject *other) {
  FractionObject *result;
  PyObject *denominator, *gcd, *numerator, *other_denominator, *other_numerator,
      *result_denominator, *result_numerator;

  gcd = _PyLong_GCD(self->numerator, other->denominator);
  if (!gcd) return NULL;
  numerator = PyNumber_FloorDivide(self->numerator, gcd);
  if (!numerator) {
    Py_DECREF(gcd);
    return NULL;
  }
  other_denominator = PyNumber_FloorDivide(other->denominator, gcd);
  Py_DECREF(gcd);
  if (!other_denominator) {
    Py_DECREF(numerator);
    return NULL;
  }

  gcd = _PyLong_GCD(other->numerator, self->denominator);
  if (!gcd) return NULL;
  other_numerator = PyNumber_FloorDivide(other->numerator, gcd);
  if (!other_numerator) {
    Py_DECREF(gcd);
    Py_DECREF(other_denominator);
    Py_DECREF(numerator);
    return NULL;
  }
  denominator = PyNumber_FloorDivide(self->denominator, gcd);
  Py_DECREF(gcd);
  if (!denominator) {
    Py_DECREF(other_numerator);
    Py_DECREF(other_denominator);
    Py_DECREF(numerator);
    return NULL;
  }

  result_numerator = PyNumber_Multiply(numerator, other_numerator);
  Py_DECREF(other_numerator);
  Py_DECREF(numerator);
  if (!result_numerator) {
    Py_DECREF(other_denominator);
    Py_DECREF(denominator);
    return NULL;
  }

  result_denominator = PyNumber_Multiply(denominator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(denominator);
  if (!result_denominator) {
    Py_DECREF(result_numerator);
    return NULL;
  }

  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(result_numerator);
    Py_DECREF(result_denominator);
    return NULL;
  }
  result->numerator = result_numerator;
  result->denominator = result_denominator;
  return result;
}

static PyObject *FractionFloat_mul(FractionObject *self, PyObject *other) {
  PyObject *result, *tmp;
  tmp = Fraction_float(self);
  if (!tmp) return NULL;
  result = PyNumber_Multiply(tmp, other);
  Py_DECREF(tmp);
  return result;
}

static FractionObject *FractionLong_mul(FractionObject *self, PyObject *other) {
  FractionObject *result;
  PyObject *gcd, *other_normalized, *result_denominator, *result_numerator;

  gcd = _PyLong_GCD(other, self->denominator);
  if (!gcd) return NULL;
  other_normalized = PyNumber_FloorDivide(other, gcd);
  if (!other_normalized) {
    Py_DECREF(gcd);
    return NULL;
  }
  result_denominator = PyNumber_FloorDivide(self->denominator, gcd);
  Py_DECREF(gcd);
  if (!result_denominator) {
    Py_DECREF(other_normalized);
    return NULL;
  }
  result_numerator = PyNumber_Multiply(self->numerator, other_normalized);
  Py_DECREF(other_normalized);
  if (!result_numerator) {
    Py_DECREF(result_denominator);
    return NULL;
  }

  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(result_numerator);
    Py_DECREF(result_denominator);
    return NULL;
  }
  result->numerator = result_numerator;
  result->denominator = result_denominator;
  return result;
}

static PyObject *Fraction_mul(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_mul((FractionObject *)self,
                                       (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)FractionLong_mul((FractionObject *)self, other);
    else if (PyFloat_Check(other))
      return (PyObject *)FractionFloat_mul((FractionObject *)self, other);
  } else if (PyLong_Check(self)) {
    return (PyObject *)FractionLong_mul((FractionObject *)other, self);
  } else if (PyFloat_Check(self)) {
    return (PyObject *)FractionFloat_mul((FractionObject *)other, self);
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static int Fraction_bool(FractionObject *self) {
  return PyObject_IsTrue(self->numerator);
}

static Py_hash_t Fraction_hash(FractionObject *self) {
  PyObject *hash_modulus, *hash_, *inverted_denominator_hash, *tmp;
  Py_hash_t result;
  hash_modulus = PyLong_FromSize_t(_PyHASH_MODULUS);
  if (!hash_modulus) return -1;
  tmp = PyLong_FromSize_t(_PyHASH_MODULUS - 2);
  if (!tmp) {
    Py_DECREF(hash_modulus);
    return -1;
  }
  inverted_denominator_hash =
      PyNumber_Power(self->denominator, tmp, hash_modulus);
  Py_DECREF(tmp);
  if (!inverted_denominator_hash) {
    Py_DECREF(hash_modulus);
    return -1;
  }
  if (PyObject_Not(inverted_denominator_hash)) {
    Py_DECREF(inverted_denominator_hash);
    Py_DECREF(hash_modulus);
    return _PyHASH_INF;
  } else {
    PyObject *numerator_modulus;
    numerator_modulus = PyNumber_Absolute(self->numerator);
    if (!numerator_modulus) {
      Py_DECREF(inverted_denominator_hash);
      Py_DECREF(hash_modulus);
      return -1;
    }
    tmp = PyNumber_Multiply(numerator_modulus, inverted_denominator_hash);
    hash_ = PyNumber_Remainder(tmp, hash_modulus);
    Py_DECREF(tmp);
    Py_DECREF(numerator_modulus);
    Py_DECREF(inverted_denominator_hash);
    Py_DECREF(hash_modulus);
    if (!hash_) return -1;
  }
  tmp = PyLong_FromLong(0);
  if (PyObject_RichCompareBool(self->numerator, tmp, Py_LT)) {
    Py_DECREF(tmp);
    tmp = hash_;
    hash_ = PyNumber_Negative(hash_);
    Py_DECREF(tmp);
    if (!hash_) return -1;
  } else
    Py_DECREF(tmp);
  result = PyLong_AsSsize_t(hash_);
  Py_DECREF(hash_);
  if (PyErr_Occurred()) return -1;
  return result == -1 ? -2 : result;
}

static PyNumberMethods Fraction_as_number = {
    .nb_absolute = (unaryfunc)Fraction_abs,
    .nb_bool = (inquiry)Fraction_bool,
    .nb_float = (unaryfunc)Fraction_float,
    .nb_multiply = (binaryfunc)Fraction_mul,
    .nb_negative = (unaryfunc)Fraction_negative,
};

static PyObject *Fraction_repr(FractionObject *self) {
  return PyUnicode_FromFormat("Fraction(%R, %R)", self->numerator,
                              self->denominator);
}

static PyTypeObject FractionType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "cfractions.Fraction",
    .tp_doc = "Represents rational numbers in the exact form.",
    .tp_basicsize = sizeof(FractionObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Fraction_new,
    .tp_init = (initproc)Fraction_init,
    .tp_dealloc = (destructor)Fraction_dealloc,
    .tp_hash = (hashfunc)Fraction_hash,
    .tp_members = Fraction_members,
    .tp_richcompare = (richcmpfunc)Fraction_richcompare,
    .tp_as_number = &Fraction_as_number,
    .tp_repr = (reprfunc)Fraction_repr,
};

static PyModuleDef _cfractions_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "cfractions",
    .m_doc = "Python C API alternative to `fractions` module.",
    .m_size = -1,
};

static int mark_as_rational(PyObject *python_type) {
  PyObject *numbers_module, *rational_interface, *tmp;
  numbers_module = PyImport_ImportModule("numbers");
  if (!numbers_module) return -1;
  rational_interface = PyObject_GetAttrString(numbers_module, "Rational");
  if (!rational_interface) {
    Py_DECREF(numbers_module);
    return -1;
  }
  PyObject *register_method_name = PyUnicode_FromString("register");
#if PY39_OR_MORE
  tmp = PyObject_CallMethodOneArg(rational_interface, register_method_name,
                                  python_type);
#else
  tmp = PyObject_CallMethodObjArgs(rational_interface, register_method_name,
                                   python_type, NULL);
#endif
  if (!tmp) {
    Py_DECREF(register_method_name);
    Py_DECREF(rational_interface);
    Py_DECREF(numbers_module);
    return -1;
  }
  Py_DECREF(tmp);
  Py_DECREF(register_method_name);
  Py_DECREF(rational_interface);
  Py_DECREF(numbers_module);
  return 0;
}

PyMODINIT_FUNC PyInit__cfractions(void) {
  PyObject *result;
  if (PyType_Ready(&FractionType) < 0) return NULL;
  result = PyModule_Create(&_cfractions_module);
  if (result == NULL) return NULL;
  Py_INCREF(&FractionType);
  if (PyModule_AddObject(result, "Fraction", (PyObject *)&FractionType) < 0) {
    Py_DECREF(&FractionType);
    Py_DECREF(result);
    return NULL;
  }
  if (mark_as_rational((PyObject *)&FractionType)) {
    Py_DECREF(result);
    return NULL;
  }
  return result;
}
