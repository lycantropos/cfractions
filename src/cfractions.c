#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include <structmember.h>

#define PY39_OR_MORE PY_VERSION_HEX >= 0x03090000

static int is_negative_Object(PyObject *self) {
  PyObject *tmp = PyLong_FromLong(0);
  int result = PyObject_RichCompareBool(self, tmp, Py_LT);
  Py_DECREF(tmp);
  return result;
}

typedef struct {
  PyObject_HEAD PyObject *numerator;
  PyObject *denominator;
} FractionObject;

static int is_negative_Fraction(FractionObject *self) {
  return is_negative_Object(self->numerator);
}

static int is_integral_Fraction(FractionObject *self) {
  PyObject *tmp = PyLong_FromLong(1);
  int result = PyObject_RichCompareBool(self->denominator, tmp, Py_EQ);
  Py_DECREF(tmp);
  return result;
}

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

static int normalize_Fraction_components_moduli(PyObject **result_numerator,
                                                PyObject **result_denominator) {
  PyObject *denominator = *result_denominator, *gcd,
           *numerator = *result_numerator, *tmp;
  gcd = _PyLong_GCD(numerator, denominator);
  if (!gcd) return -1;
  tmp = PyLong_FromLong(1);
  if (PyObject_RichCompareBool(gcd, tmp, Py_NE)) {
    Py_DECREF(tmp);
    numerator = PyNumber_FloorDivide(numerator, gcd);
    if (!numerator) {
      Py_DECREF(gcd);
      return -1;
    }
    denominator = PyNumber_FloorDivide(denominator, gcd);
    if (!denominator) {
      Py_DECREF(numerator);
      Py_DECREF(gcd);
      return -1;
    }
    tmp = *result_numerator;
    *result_numerator = numerator;
    Py_DECREF(tmp);
    tmp = *result_denominator;
    *result_denominator = denominator;
    Py_DECREF(tmp);
  } else
    Py_DECREF(tmp);
  Py_DECREF(gcd);
  return 0;
}

static int normalize_Fraction_components_signs(PyObject **result_numerator,
                                               PyObject **result_denominator) {
  PyObject *denominator = *result_denominator, *numerator = *result_numerator,
           *tmp;
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
    tmp = *result_numerator;
    *result_numerator = numerator;
    Py_DECREF(tmp);
    tmp = *result_denominator;
    *result_denominator = denominator;
    Py_DECREF(tmp);
  } else
    Py_DECREF(tmp);
  return 0;
}

static int Fraction_init(FractionObject *self, PyObject *args) {
  PyObject *numerator = NULL, *denominator = NULL, *tmp;
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

    if (normalize_Fraction_components_moduli(&numerator, &denominator) < 0) {
      Py_INCREF(numerator);
      Py_INCREF(denominator);
      return -1;
    }

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
    } else if (PyObject_TypeCheck(numerator, &FractionType)) {
      FractionObject* fraction_numerator = (FractionObject *)numerator;
      tmp = self->denominator;
      Py_INCREF(fraction_numerator->denominator);
      self->denominator = fraction_numerator->denominator;
      Py_XDECREF(tmp);
      Py_INCREF(fraction_numerator->numerator);
      numerator = fraction_numerator->numerator;
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
  if (is_negative_Fraction(self))
    result = Fraction_negative(self);
  else {
    Py_INCREF((PyObject *)self);
    result = self;
  }
  return result;
}

static PyObject *Fraction_float(FractionObject *self) {
  return PyNumber_TrueDivide(self->numerator, self->denominator);
}

static FractionObject *Fractions_add(FractionObject *self,
                                     FractionObject *other) {
  FractionObject *result;
  PyObject *denominator, *numerator, *first_numerator_component,
      *second_numerator_component;
  first_numerator_component =
      PyNumber_Multiply(self->numerator, other->denominator);
  if (!first_numerator_component) return NULL;
  second_numerator_component =
      PyNumber_Multiply(other->numerator, self->denominator);
  if (!second_numerator_component) {
    Py_DECREF(first_numerator_component);
    return NULL;
  }
  numerator =
      PyNumber_Add(first_numerator_component, second_numerator_component);
  Py_DECREF(second_numerator_component);
  Py_DECREF(first_numerator_component);
  if (!numerator) return NULL;
  denominator = PyNumber_Multiply(self->denominator, other->denominator);
  if (!denominator) {
    Py_DECREF(numerator);
    return NULL;
  }
  if (normalize_Fraction_components_moduli(&numerator, &denominator)) {
    Py_DECREF(denominator);
    Py_DECREF(numerator);
    return NULL;
  }
  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(denominator);
    Py_DECREF(numerator);
    return NULL;
  }
  result->numerator = numerator;
  result->denominator = denominator;
  return result;
}

static PyObject *FractionFloat_add(FractionObject *self, PyObject *other) {
  PyObject *result, *tmp;
  tmp = Fraction_float(self);
  if (!tmp) return NULL;
  result = PyNumber_Add(tmp, other);
  Py_DECREF(tmp);
  return result;
}

static FractionObject *FractionLong_add(FractionObject *self, PyObject *other) {
  FractionObject *result;
  PyObject *result_denominator, *result_numerator, *tmp;
  tmp = PyNumber_Multiply(other, self->denominator);
  if (!tmp) return NULL;
  result_numerator = PyNumber_Add(self->numerator, tmp);
  Py_DECREF(tmp);
  Py_INCREF(self->denominator);
  result_denominator = self->denominator;
  if (normalize_Fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
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

static PyObject *Fraction_add(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_add((FractionObject *)self,
                                       (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)FractionLong_add((FractionObject *)self, other);
    else if (PyFloat_Check(other))
      return (PyObject *)FractionFloat_add((FractionObject *)self, other);
  } else if (PyLong_Check(self))
    return (PyObject *)FractionLong_add((FractionObject *)other, self);
  else if (PyFloat_Check(self))
    return (PyObject *)FractionFloat_add((FractionObject *)other, self);
  Py_RETURN_NOTIMPLEMENTED;
}

static int Fraction_bool(FractionObject *self) {
  return PyObject_IsTrue(self->numerator);
}

static PyObject *Fraction_ceil_impl(FractionObject *self) {
  PyObject *result, *tmp;
  tmp = PyNumber_Negative(self->numerator);
  if (!tmp) return NULL;
  result = PyNumber_FloorDivide(tmp, self->denominator);
  Py_DECREF(tmp);
  if (!result) return NULL;
  tmp = result;
  result = PyNumber_Negative(result);
  Py_DECREF(tmp);
  return result;
}

static PyObject *Fraction_ceil(FractionObject *self, PyObject *args) {
  return Fraction_ceil_impl(self);
}

static PyObject *Fraction_floor_impl(FractionObject *self) {
  return PyNumber_FloorDivide(self->numerator, self->denominator);
}

static PyObject *Fraction_floor(FractionObject *self, PyObject *args) {
  return Fraction_floor_impl(self);
}

static PyObject *Fractions_floor_divide(FractionObject *self,
                                        FractionObject *other) {
  PyObject *denominator, *divisor, *dividend, *gcd, *numerator,
      *other_denominator, *other_numerator, *result;
  gcd = _PyLong_GCD(self->numerator, other->numerator);
  if (!gcd) return NULL;
  numerator = PyNumber_FloorDivide(self->numerator, gcd);
  if (!numerator) {
    Py_DECREF(gcd);
    return NULL;
  }
  other_numerator = PyNumber_FloorDivide(other->numerator, gcd);
  Py_DECREF(gcd);
  if (!other_numerator) {
    Py_DECREF(numerator);
    return NULL;
  }
  gcd = _PyLong_GCD(other->denominator, self->denominator);
  if (!gcd) return NULL;
  denominator = PyNumber_FloorDivide(self->denominator, gcd);
  if (!denominator) {
    Py_DECREF(gcd);
    Py_DECREF(other_numerator);
    Py_DECREF(numerator);
    return NULL;
  }
  other_denominator = PyNumber_FloorDivide(other->denominator, gcd);
  Py_DECREF(gcd);
  if (!other_denominator) {
    Py_DECREF(denominator);
    Py_DECREF(other_numerator);
    Py_DECREF(numerator);
    return NULL;
  }
  dividend = PyNumber_Multiply(numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(numerator);
  if (!dividend) {
    Py_DECREF(other_numerator);
    Py_DECREF(denominator);
    return NULL;
  }
  divisor = PyNumber_Multiply(denominator, other_numerator);
  Py_DECREF(other_numerator);
  Py_DECREF(denominator);
  if (!divisor) {
    Py_DECREF(dividend);
    return NULL;
  }
  result = PyNumber_FloorDivide(dividend, divisor);
  Py_DECREF(dividend);
  Py_DECREF(divisor);
  return result;
}

static PyObject *FractionLong_floor_divide(FractionObject *self,
                                           PyObject *other) {
  PyObject *divisor, *dividend, *gcd, *other_normalized, *result;
  gcd = _PyLong_GCD(self->numerator, other);
  if (!gcd) return NULL;
  dividend = PyNumber_FloorDivide(self->numerator, gcd);
  if (!dividend) {
    Py_DECREF(gcd);
    return NULL;
  }
  other_normalized = PyNumber_FloorDivide(other, gcd);
  Py_DECREF(gcd);
  if (!other_normalized) {
    Py_DECREF(dividend);
    return NULL;
  }
  divisor = PyNumber_Multiply(self->denominator, other_normalized);
  Py_DECREF(other_normalized);
  if (!divisor) {
    Py_DECREF(dividend);
    return NULL;
  }
  result = PyNumber_FloorDivide(dividend, divisor);
  Py_DECREF(dividend);
  Py_DECREF(divisor);
  return result;
}

static PyObject *LongFraction_floor_divide(PyObject *self,
                                           FractionObject *other) {
  PyObject *divisor, *dividend, *gcd, *self_normalized, *result;
  gcd = _PyLong_GCD(self, other->numerator);
  if (!gcd) return NULL;
  divisor = PyNumber_FloorDivide(other->numerator, gcd);
  if (!divisor) {
    Py_DECREF(gcd);
    return NULL;
  }
  self_normalized = PyNumber_FloorDivide(self, gcd);
  Py_DECREF(gcd);
  if (!self_normalized) {
    Py_DECREF(divisor);
    return NULL;
  }
  dividend = PyNumber_Multiply(self_normalized, other->denominator);
  Py_DECREF(self_normalized);
  if (!dividend) {
    Py_DECREF(divisor);
    return NULL;
  }
  result = PyNumber_FloorDivide(dividend, divisor);
  Py_DECREF(dividend);
  Py_DECREF(divisor);
  return result;
}

static PyObject *Fraction_floor_divide(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_floor_divide((FractionObject *)self,
                                                (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)FractionLong_floor_divide((FractionObject *)self,
                                                   other);
    else if (PyFloat_Check(other)) {
      PyObject *result, *tmp;
      tmp = Fraction_float((FractionObject *)self);
      if (!tmp) return NULL;
      result = PyNumber_FloorDivide(tmp, other);
      Py_DECREF(tmp);
      return result;
    }
  } else if (PyLong_Check(self))
    return (PyObject *)LongFraction_floor_divide(self, (FractionObject *)other);
  else if (PyFloat_Check(self)) {
    PyObject *result, *tmp;
    tmp = Fraction_float((FractionObject *)other);
    if (!tmp) return NULL;
    result = PyNumber_FloorDivide(self, tmp);
    Py_DECREF(tmp);
    return result;
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *Fractions_multiply(FractionObject *self,
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

static PyObject *Fraction_trunc(FractionObject *self, PyObject *args) {
  return is_negative_Fraction(self) ? Fraction_ceil_impl(self)
                                    : Fraction_floor_impl(self);
}

static PyObject *FractionFloat_multiply(FractionObject *self, PyObject *other) {
  PyObject *result, *tmp;
  tmp = Fraction_float(self);
  if (!tmp) return NULL;
  result = PyNumber_Multiply(tmp, other);
  Py_DECREF(tmp);
  return result;
}

static FractionObject *FractionLong_multiply(FractionObject *self,
                                             PyObject *other) {
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

static PyObject *Fraction_multiply(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_multiply((FractionObject *)self,
                                            (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)FractionLong_multiply((FractionObject *)self, other);
    else if (PyFloat_Check(other))
      return (PyObject *)FractionFloat_multiply((FractionObject *)self, other);
  } else if (PyLong_Check(self))
    return (PyObject *)FractionLong_multiply((FractionObject *)other, self);
  else if (PyFloat_Check(self))
    return (PyObject *)FractionFloat_multiply((FractionObject *)other, self);
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *Fractions_remainder(FractionObject *self,
                                           FractionObject *other) {
  FractionObject *result;
  PyObject *dividend, *divisor, *result_denominator, *result_numerator;
  dividend = PyNumber_Multiply(self->numerator, other->denominator);
  if (!dividend) return NULL;
  divisor = PyNumber_Multiply(other->numerator, self->denominator);
  if (!divisor) {
    Py_DECREF(dividend);
    return NULL;
  }
  result_numerator = PyNumber_Remainder(dividend, divisor);
  Py_DECREF(dividend);
  Py_DECREF(divisor);
  if (!result_numerator) return NULL;
  result_denominator = PyNumber_Multiply(self->denominator, other->denominator);
  if (!result_denominator) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  if (normalize_Fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  result->numerator = result_numerator;
  result->denominator = result_denominator;
  return result;
}

static FractionObject *FractionLong_remainder(FractionObject *self,
                                              PyObject *other) {
  FractionObject *result;
  PyObject *result_denominator, *result_numerator, *tmp;
  tmp = PyNumber_Multiply(other, self->denominator);
  if (!tmp) return NULL;
  result_numerator = PyNumber_Remainder(self->numerator, tmp);
  Py_DECREF(tmp);
  if (!result_numerator) return NULL;
  Py_INCREF(self->denominator);
  result_denominator = self->denominator;
  if (normalize_Fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  result->numerator = result_numerator;
  result->denominator = result_denominator;
  return result;
}

static FractionObject *LongFraction_remainder(PyObject *self,
                                              FractionObject *other) {
  FractionObject *result;
  PyObject *result_denominator, *result_numerator, *tmp;
  tmp = PyNumber_Multiply(self, other->denominator);
  if (!tmp) return NULL;
  result_numerator = PyNumber_Remainder(tmp, other->numerator);
  Py_DECREF(tmp);
  if (!result_numerator) return NULL;
  Py_INCREF(other->denominator);
  result_denominator = other->denominator;
  if (normalize_Fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
  }
  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  result->numerator = result_numerator;
  result->denominator = result_denominator;
  return result;
}

static PyObject *FractionObject_remainder(FractionObject *self,
                                          PyObject *other) {
  if (PyObject_TypeCheck(other, &FractionType))
    return (PyObject *)Fractions_remainder(self, (FractionObject *)other);
  else if (PyLong_Check(other))
    return (PyObject *)FractionLong_remainder(self, other);
  else if (PyFloat_Check(other)) {
    PyObject *result, *tmp;
    tmp = Fraction_float(self);
    result = PyNumber_Remainder(tmp, other);
    Py_DECREF(tmp);
    return result;
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static PyObject *Fraction_remainder(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    return FractionObject_remainder((FractionObject *)self, other);
  } else if (PyLong_Check(self)) {
    return (PyObject *)LongFraction_remainder(self, (FractionObject *)other);
  } else if (PyFloat_Check(self)) {
    PyObject *result, *tmp;
    tmp = Fraction_float((FractionObject *)other);
    result = PyNumber_Remainder(self, tmp);
    Py_DECREF(tmp);
    return result;
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static PyObject *LongFraction_power(PyObject *self, FractionObject *exponent,
                                    PyObject *modulo) {
  if (is_integral_Fraction(exponent)) {
    FractionObject *result;
    PyObject *result_numerator, *result_denominator;
    if (is_negative_Fraction(exponent)) {
      if (PyObject_Not(self)) {
        PyErr_SetString(PyExc_ZeroDivisionError,
                        "Either exponent should be non-negative "
                        "or base should not be zero.");
        return NULL;
      }
      PyObject *positive_exponent = PyNumber_Negative(exponent->numerator);
      if (!positive_exponent) return NULL;
      result_denominator = PyNumber_Power(self, positive_exponent, Py_None);
      Py_DECREF(positive_exponent);
      if (!result_denominator) return NULL;
      result_numerator = PyLong_FromLong(1);
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
      if (modulo != Py_None) {
        PyObject *remainder = FractionObject_remainder(result, modulo);
        Py_DECREF(result);
        return remainder;
      }
      return (PyObject *)result;
    } else {
      result_numerator = PyNumber_Power(self, exponent->numerator, modulo);
      if (!result_numerator) return NULL;
      result_denominator = PyLong_FromLong(1);
      if (!result_denominator) {
        Py_DECREF(result_numerator);
        return NULL;
      }
      result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
      if (!result) {
        Py_DECREF(result_denominator);
        Py_DECREF(result_numerator);
        return NULL;
      }
      result->numerator = result_numerator;
      result->denominator = result_denominator;
      return (PyObject *)result;
    }
  } else {
    PyObject *float_exponent, *result;
    float_exponent = Fraction_float(exponent);
    if (!float_exponent) return NULL;
    result = PyNumber_Power(self, float_exponent, modulo);
    Py_DECREF(float_exponent);
    return result;
  }
}

static PyObject *FractionLong_power(FractionObject *self, PyObject *exponent,
                                    PyObject *modulo) {
  if (is_negative_Object(exponent)) {
    if (!Fraction_bool(self)) {
      PyErr_SetString(PyExc_ZeroDivisionError,
                      "Either exponent should be non-negative "
                      "or base should not be zero.");
      return NULL;
    }
    PyObject *inverted_denominator, *inverted_numerator, *positive_exponent,
        *result;
    FractionObject *inverted;
    positive_exponent = PyNumber_Negative(exponent);
    if (!positive_exponent) return NULL;
    Py_INCREF(self->denominator);
    inverted_numerator = self->denominator;
    Py_INCREF(self->numerator);
    inverted_denominator = self->numerator;
    if (normalize_Fraction_components_signs(&inverted_numerator,
                                            &inverted_denominator) < 0) {
      Py_DECREF(positive_exponent);
      return NULL;
    }
    inverted = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
    if (!inverted) {
      Py_DECREF(positive_exponent);
      return NULL;
    }
    inverted->numerator = inverted_numerator;
    inverted->denominator = inverted_denominator;
    result = FractionLong_power(inverted, positive_exponent, modulo);
    Py_DECREF(inverted);
    Py_DECREF(positive_exponent);
    return result;
  }
  if (is_integral_Fraction(self) &&
      (modulo == Py_None || PyLong_Check(modulo))) {
    PyObject *result_numerator, *result_denominator;
    FractionObject *result;
    result_numerator = PyNumber_Power(self->numerator, exponent, modulo);
    if (!result_numerator) return NULL;
    result_denominator = PyLong_FromLong(1);
    if (!result_denominator) {
      Py_DECREF(result_numerator);
      return NULL;
    }
    result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
    if (!result) {
      Py_DECREF(result_denominator);
      Py_DECREF(result_numerator);
      return NULL;
    }
    result->numerator = result_numerator;
    result->denominator = result_denominator;
    return (PyObject *)result;
  } else {
    PyObject *result_numerator, *result_denominator;
    FractionObject *result;
    result_numerator = PyNumber_Power(self->numerator, exponent, Py_None);
    if (!result_numerator) return NULL;
    result_denominator = PyNumber_Power(self->denominator, exponent, Py_None);
    if (!result_denominator) {
      Py_DECREF(result_numerator);
      return NULL;
    }
    result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
    if (!result) {
      Py_DECREF(result_denominator);
      Py_DECREF(result_numerator);
      return NULL;
    }
    result->numerator = result_numerator;
    result->denominator = result_denominator;
    if (modulo != Py_None) {
      PyObject *remainder = FractionObject_remainder(result, modulo);
      Py_DECREF(result);
      return remainder;
    }
    return (PyObject *)result;
  }
}

static PyObject *FloatFraction_power(PyObject *self, FractionObject *exponent,
                                     PyObject *modulo) {
  PyObject *float_exponent;
  float_exponent = Fraction_float(exponent);
  return !float_exponent ? NULL : PyNumber_Power(self, float_exponent, modulo);
}

static PyObject *Fractions_power(FractionObject *self, FractionObject *exponent,
                                 PyObject *modulo) {
  if (is_integral_Fraction(exponent))
    return FractionLong_power(self, exponent->numerator, modulo);
  else {
    PyObject *float_self, *result;
    float_self = Fraction_float(self);
    if (!float_self) return NULL;
    result = FloatFraction_power(float_self, exponent, modulo);
    Py_DECREF(float_self);
    return result;
  }
}

static PyObject *Fraction_power(PyObject *self, PyObject *exponent,
                                PyObject *modulo) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(exponent, &FractionType))
      return Fractions_power((FractionObject *)self, (FractionObject *)exponent,
                             modulo);
    else if (PyLong_Check(exponent))
      return FractionLong_power((FractionObject *)self, exponent, modulo);
    else if (PyFloat_Check(exponent)) {
      PyObject *float_self, *result;
      float_self = Fraction_float((FractionObject *)self);
      result = PyNumber_Power(float_self, exponent, modulo);
      Py_DECREF(float_self);
      return result;
    }
  } else if (PyObject_TypeCheck(exponent, &FractionType)) {
    if (PyLong_Check(self))
      return LongFraction_power(self, (FractionObject *)exponent, modulo);
    else if (PyFloat_Check(self))
      return FloatFraction_power(self, (FractionObject *)exponent, modulo);
  } else {
    PyObject *result, *tmp;
    tmp = PyNumber_Power(self, exponent, Py_None);
    if (!tmp) return NULL;
    result = Fraction_remainder(tmp, modulo);
    Py_DECREF(tmp);
    return result;
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *Fractions_subtract(FractionObject *self,
                                          FractionObject *other) {
  FractionObject *result;
  PyObject *denominator, *numerator, *first_numerator_component,
      *second_numerator_component;
  first_numerator_component =
      PyNumber_Multiply(self->numerator, other->denominator);
  if (!first_numerator_component) return NULL;
  second_numerator_component =
      PyNumber_Multiply(other->numerator, self->denominator);
  if (!second_numerator_component) {
    Py_DECREF(first_numerator_component);
    return NULL;
  }
  numerator =
      PyNumber_Subtract(first_numerator_component, second_numerator_component);
  Py_DECREF(second_numerator_component);
  Py_DECREF(first_numerator_component);
  if (!numerator) return NULL;
  denominator = PyNumber_Multiply(self->denominator, other->denominator);
  if (!denominator) {
    Py_DECREF(numerator);
    return NULL;
  }
  if (normalize_Fraction_components_moduli(&numerator, &denominator)) {
    Py_DECREF(denominator);
    Py_DECREF(numerator);
    return NULL;
  }
  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(denominator);
    Py_DECREF(numerator);
    return NULL;
  }
  result->numerator = numerator;
  result->denominator = denominator;
  return result;
}

static PyObject *FractionFloat_subtract(FractionObject *self, PyObject *other) {
  PyObject *result, *tmp;
  tmp = Fraction_float(self);
  if (!tmp) return NULL;
  result = PyNumber_Subtract(tmp, other);
  Py_DECREF(tmp);
  return result;
}

static FractionObject *FractionLong_subtract(FractionObject *self,
                                             PyObject *other) {
  FractionObject *result;
  PyObject *result_denominator, *result_numerator, *tmp;
  tmp = PyNumber_Multiply(other, self->denominator);
  if (!tmp) return NULL;
  result_numerator = PyNumber_Subtract(self->numerator, tmp);
  Py_DECREF(tmp);
  Py_INCREF(self->denominator);
  result_denominator = self->denominator;
  if (normalize_Fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
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

static PyObject *Fraction_subtract(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_subtract((FractionObject *)self,
                                            (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)FractionLong_subtract((FractionObject *)self, other);
    else if (PyFloat_Check(other))
      return (PyObject *)FractionFloat_subtract((FractionObject *)self, other);
  } else if (PyLong_Check(self)) {
    FractionObject *result =
        FractionLong_subtract((FractionObject *)other, self);
    PyObject *tmp;
    tmp = result->numerator;
    result->numerator = PyNumber_Negative(result->numerator);
    Py_DECREF(tmp);
    return (PyObject *)result;
  } else if (PyFloat_Check(self)) {
    PyObject *result, *tmp;
    tmp = (PyObject *)FractionFloat_subtract((FractionObject *)other, self);
    result = PyNumber_Negative(tmp);
    Py_DECREF(tmp);
    return result;
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *Fractions_true_divide(FractionObject *self,
                                             FractionObject *other) {
  if (!Fraction_bool(other)) {
    PyErr_Format(PyExc_ZeroDivisionError, "Fraction(%S, 0)", self->numerator);
    return NULL;
  }
  FractionObject *result;
  PyObject *denominator, *gcd, *numerator, *other_denominator, *other_numerator,
      *result_denominator, *result_numerator;

  gcd = _PyLong_GCD(self->numerator, other->numerator);
  if (!gcd) return NULL;
  numerator = PyNumber_FloorDivide(self->numerator, gcd);
  if (!numerator) {
    Py_DECREF(gcd);
    return NULL;
  }
  other_numerator = PyNumber_FloorDivide(other->numerator, gcd);
  Py_DECREF(gcd);
  if (!other_numerator) {
    Py_DECREF(numerator);
    return NULL;
  }

  gcd = _PyLong_GCD(other->denominator, self->denominator);
  if (!gcd) return NULL;
  denominator = PyNumber_FloorDivide(self->denominator, gcd);
  if (!denominator) {
    Py_DECREF(gcd);
    Py_DECREF(other_numerator);
    Py_DECREF(numerator);
    return NULL;
  }
  other_denominator = PyNumber_FloorDivide(other->denominator, gcd);
  Py_DECREF(gcd);
  if (!other_denominator) {
    Py_DECREF(denominator);
    Py_DECREF(other_numerator);
    Py_DECREF(numerator);
    return NULL;
  }

  result_numerator = PyNumber_Multiply(numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(numerator);
  if (!result_numerator) {
    Py_DECREF(other_numerator);
    Py_DECREF(denominator);
    return NULL;
  }
  result_denominator = PyNumber_Multiply(denominator, other_numerator);
  Py_DECREF(other_numerator);
  Py_DECREF(denominator);
  if (!result_denominator) {
    Py_DECREF(result_numerator);
    return NULL;
  }

  if (normalize_Fraction_components_signs(&result_numerator,
                                          &result_denominator) < 0) {
    Py_INCREF(result_denominator);
    Py_INCREF(result_numerator);
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

static FractionObject *Fraction_positive(FractionObject *self) {
  Py_INCREF((PyObject *)self);
  return self;
}

static FractionObject *FractionLong_true_divide(FractionObject *self,
                                                PyObject *other) {
  if (PyObject_Not(other)) {
    PyErr_Format(PyExc_ZeroDivisionError, "Fraction(%S, 0)", self->numerator);
    return NULL;
  }
  PyObject *gcd, *result_denominator, *result_numerator, *other_normalized;
  FractionObject *result;

  gcd = _PyLong_GCD(self->numerator, other);
  if (!gcd) return NULL;
  result_numerator = PyNumber_FloorDivide(self->numerator, gcd);
  if (!result_numerator) {
    Py_DECREF(gcd);
    return NULL;
  }
  other_normalized = PyNumber_FloorDivide(other, gcd);
  Py_DECREF(gcd);
  if (!other_normalized) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  result_denominator = PyNumber_Multiply(self->denominator, other_normalized);
  Py_DECREF(other_normalized);
  if (!result_denominator) {
    Py_DECREF(result_numerator);
    return NULL;
  }

  if (normalize_Fraction_components_signs(&result_numerator,
                                          &result_denominator) < 0) {
    Py_INCREF(result_denominator);
    Py_INCREF(result_numerator);
    return NULL;
  }

  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  result->numerator = result_numerator;
  result->denominator = result_denominator;
  return result;
}

static FractionObject *LongFraction_true_divide(PyObject *self,
                                                FractionObject *other) {
  if (!Fraction_bool(other)) {
    PyErr_Format(PyExc_ZeroDivisionError, "Fraction(%S, 0)", self);
    return NULL;
  }
  PyObject *gcd, *result_denominator, *result_numerator, *self_normalized;
  FractionObject *result;

  gcd = _PyLong_GCD(self, other->numerator);
  if (!gcd) return NULL;
  result_denominator = PyNumber_FloorDivide(other->numerator, gcd);
  if (!result_denominator) {
    Py_DECREF(gcd);
    return NULL;
  }
  self_normalized = PyNumber_FloorDivide(self, gcd);
  Py_DECREF(gcd);
  if (!self_normalized) {
    Py_DECREF(result_denominator);
    return NULL;
  }
  result_numerator = PyNumber_Multiply(self_normalized, other->denominator);
  Py_DECREF(self_normalized);
  if (!result_numerator) {
    Py_DECREF(result_denominator);
    return NULL;
  }

  if (normalize_Fraction_components_signs(&result_numerator,
                                          &result_denominator) < 0) {
    Py_INCREF(result_denominator);
    Py_INCREF(result_numerator);
    return NULL;
  }

  result = PyObject_New(FractionObject, (PyTypeObject *)&FractionType);
  if (!result) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  result->numerator = result_numerator;
  result->denominator = result_denominator;
  return result;
}

static PyObject *Fraction_true_divide(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_true_divide((FractionObject *)self,
                                               (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)FractionLong_true_divide((FractionObject *)self,
                                                  other);
    else if (PyFloat_Check(other)) {
      PyObject *result, *tmp;
      tmp = Fraction_float((FractionObject *)self);
      if (!tmp) return NULL;
      result = PyNumber_TrueDivide(tmp, other);
      Py_DECREF(tmp);
      return result;
    }
  } else if (PyLong_Check(self))
    return (PyObject *)LongFraction_true_divide(self, (FractionObject *)other);
  else if (PyFloat_Check(self)) {
    PyObject *result, *tmp;
    tmp = Fraction_float((FractionObject *)other);
    if (!tmp) return NULL;
    result = PyNumber_TrueDivide(self, tmp);
    Py_DECREF(tmp);
    return result;
  }
  Py_RETURN_NOTIMPLEMENTED;
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
  if (is_negative_Fraction(self)) {
    tmp = hash_;
    hash_ = PyNumber_Negative(hash_);
    Py_DECREF(tmp);
    if (!hash_) return -1;
  }
  result = PyLong_AsSsize_t(hash_);
  Py_DECREF(hash_);
  if (PyErr_Occurred()) return -1;
  return result == -1 ? -2 : result;
}

static PyObject *Fraction_repr(FractionObject *self) {
  return PyUnicode_FromFormat("Fraction(%R, %R)", self->numerator,
                              self->denominator);
}

static PyMemberDef Fraction_members[] = {
    {"numerator", T_OBJECT_EX, offsetof(FractionObject, numerator), READONLY,
     "Numerator of the fraction."},
    {"denominator", T_OBJECT_EX, offsetof(FractionObject, denominator),
     READONLY, "Denominator of the fraction."},
    {NULL} /* sentinel */
};

static PyMethodDef Fraction_methods[] = {
    {"__ceil__", (PyCFunction)Fraction_ceil, METH_NOARGS, NULL},
    {"__floor__", (PyCFunction)Fraction_floor, METH_NOARGS, NULL},
    {"__trunc__", (PyCFunction)Fraction_trunc, METH_NOARGS, NULL},
    {NULL, NULL} /* sentinel */
};

static PyNumberMethods Fraction_as_number = {
    .nb_absolute = (unaryfunc)Fraction_abs,
    .nb_add = Fraction_add,
    .nb_bool = (inquiry)Fraction_bool,
    .nb_float = (unaryfunc)Fraction_float,
    .nb_floor_divide = Fraction_floor_divide,
    .nb_multiply = Fraction_multiply,
    .nb_negative = (unaryfunc)Fraction_negative,
    .nb_positive = (unaryfunc)Fraction_positive,
    .nb_power = Fraction_power,
    .nb_remainder = Fraction_remainder,
    .nb_subtract = Fraction_subtract,
    .nb_true_divide = Fraction_true_divide,
};

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
    .tp_methods = Fraction_methods,
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
  if (mark_as_rational((PyObject *)&FractionType) < 0) {
    Py_DECREF(result);
    return NULL;
  }
  return result;
}
