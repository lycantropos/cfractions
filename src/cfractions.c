#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include <structmember.h>

#define PY3_9_OR_MORE PY_VERSION_HEX >= 0x03090000
#define PY3_11_OR_MORE PY_VERSION_HEX >= 0x030b0000

static int is_negative_py_object(PyObject *self) {
  PyObject *tmp = PyLong_FromLong(0);
  int result = PyObject_RichCompareBool(self, tmp, Py_LT);
  Py_DECREF(tmp);
  return result;
}

static int is_unit_py_object_bool(PyObject *self) {
  PyObject *tmp = PyLong_FromLong(1);
  if (tmp == NULL) return -1;
  int result = PyObject_RichCompareBool(self, tmp, Py_EQ);
  Py_DECREF(tmp);
  return result;
}

static PyObject *is_unit_py_object(PyObject *self) {
  PyObject *tmp = PyLong_FromLong(1);
  if (tmp == NULL) return NULL;
  PyObject *result = PyObject_RichCompare(self, tmp, Py_EQ);
  Py_DECREF(tmp);
  return result;
}

static PyObject *round_py_object(PyObject *self) {
  PyObject *round_method_name = PyUnicode_FromString("__round__");
  if (round_method_name == NULL) return NULL;
  PyObject *result =
#if PY3_9_OR_MORE
      PyObject_CallMethodNoArgs(self, round_method_name)
#else
      PyObject_CallMethodObjArgs(self, round_method_name, NULL)
#endif
      ;
  Py_DECREF(round_method_name);
  return result;
}

static int py_unicode_is_ascii(PyObject *self) {
  return ((PyASCIIObject *)self)->state.ascii;
}

static PyObject *Rational = NULL;

typedef struct {
  PyObject_HEAD PyObject *numerator;
  PyObject *denominator;
} FractionObject;

static int is_negative_fraction(FractionObject *self) {
  return is_negative_py_object(self->numerator);
}

static int is_integral_fraction(FractionObject *self) {
  return is_unit_py_object_bool(self->denominator);
}

static void fraction_dealloc(FractionObject *self) {
  Py_DECREF(self->numerator);
  Py_DECREF(self->denominator);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject FractionType;

static int normalize_fraction_components_moduli(PyObject **result_numerator,
                                                PyObject **result_denominator) {
  PyObject *gcd = _PyLong_GCD(*result_numerator, *result_denominator);
  if (gcd == NULL) return -1;
  int is_gcd_unit = is_unit_py_object_bool(gcd);
  if (is_gcd_unit < 0) {
    Py_DECREF(gcd);
    return -1;
  } else if (!is_gcd_unit) {
    PyObject *numerator = PyNumber_FloorDivide(*result_numerator, gcd);
    if (numerator == NULL) {
      Py_DECREF(gcd);
      return -1;
    }
    PyObject *denominator = PyNumber_FloorDivide(*result_denominator, gcd);
    if (denominator == NULL) {
      Py_DECREF(numerator);
      Py_DECREF(gcd);
      return -1;
    }
    PyObject *tmp = *result_numerator;
    *result_numerator = numerator;
    Py_DECREF(tmp);
    tmp = *result_denominator;
    *result_denominator = denominator;
    Py_DECREF(tmp);
  }
  Py_DECREF(gcd);
  return 0;
}

static int normalize_fraction_components_signs(PyObject **result_numerator,
                                               PyObject **result_denominator) {
  int is_denominator_negative = is_negative_py_object(*result_denominator);
  if (is_denominator_negative < 0)
    return -1;
  else if (is_denominator_negative) {
    PyObject *numerator = PyNumber_Negative(*result_numerator);
    if (numerator == NULL) return -1;
    PyObject *denominator = PyNumber_Negative(*result_denominator);
    if (denominator == NULL) {
      Py_DECREF(numerator);
      return -1;
    }
    PyObject *tmp = *result_numerator;
    *result_numerator = numerator;
    Py_DECREF(tmp);
    tmp = *result_denominator;
    *result_denominator = denominator;
    Py_DECREF(tmp);
  }
  return 0;
}

static int parse_fraction_components_from_rational(
    PyObject *rational, PyObject **result_numerator,
    PyObject **result_denominator) {
  PyObject *numerator = PyObject_GetAttrString(rational, "numerator");
  if (numerator == NULL) return -1;
  PyObject *tmp = numerator;
  numerator = PyNumber_Long(numerator);
  Py_DECREF(tmp);
  if (numerator == NULL) return -1;
  PyObject *denominator = PyObject_GetAttrString(rational, "denominator");
  if (denominator == NULL) {
    Py_DECREF(numerator);
    return -1;
  }
  tmp = denominator;
  denominator = PyNumber_Long(denominator);
  Py_DECREF(tmp);
  if (denominator == NULL) {
    Py_DECREF(numerator);
    return -1;
  }
  if (normalize_fraction_components_signs(&numerator, &denominator) < 0 ||
      normalize_fraction_components_moduli(&numerator, &denominator) < 0) {
    Py_DECREF(denominator);
    Py_DECREF(numerator);
    return -1;
  }
  *result_numerator = numerator;
  *result_denominator = denominator;
  return 0;
}

static int parse_fraction_components_from_double(
    double value, PyObject **result_numerator, PyObject **result_denominator) {
  if (isinf(value)) {
    PyErr_SetString(PyExc_OverflowError,
                    "Cannot construct Fraction from infinity.");
    return -1;
  }
  if (isnan(value)) {
    PyErr_SetString(PyExc_ValueError, "Cannot construct Fraction from NaN.");
    return -1;
  }
  int exponent;
  value = frexp(value, &exponent);
  for (size_t index = 0; index < 300 && value != floor(value); ++index) {
    value *= 2.0;
    exponent--;
  }
  PyObject *numerator = PyLong_FromDouble(value);
  if (numerator == NULL) return -1;
  PyObject *denominator = PyLong_FromLong(1);
  if (denominator == NULL) {
    Py_DECREF(numerator);
    return -1;
  }
  PyObject *exponent_object = PyLong_FromLong(abs(exponent));
  if (exponent_object == NULL) {
    Py_DECREF(numerator);
    Py_DECREF(denominator);
    return -1;
  }
  if (exponent > 0) {
    PyObject *tmp = numerator;
    numerator = PyNumber_Lshift(numerator, exponent_object);
    Py_DECREF(tmp);
    if (numerator == NULL) {
      Py_DECREF(denominator);
      Py_DECREF(exponent_object);
      return -1;
    }
  } else {
    PyObject *tmp = denominator;
    denominator = PyNumber_Lshift(denominator, exponent_object);
    Py_DECREF(tmp);
    if (denominator == NULL) {
      Py_DECREF(numerator);
      Py_DECREF(exponent_object);
      return -1;
    }
  }
  Py_DECREF(exponent_object);
  *result_denominator = denominator;
  *result_numerator = numerator;
  return 0;
}

const Py_UCS1 ascii_whitespaces[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    /*     case 0x0009: * CHARACTER TABULATION */
    /*     case 0x000A: * LINE FEED */
    /*     case 0x000B: * LINE TABULATION */
    /*     case 0x000C: * FORM FEED */
    /*     case 0x000D: * CARRIAGE RETURN */
    0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*     case 0x001C: * FILE SEPARATOR */
    /*     case 0x001D: * GROUP SEPARATOR */
    /*     case 0x001E: * RECORD SEPARATOR */
    /*     case 0x001F: * UNIT SEPARATOR */
    0, 0, 0, 0, 1, 1, 1, 1,
    /*     case 0x0020: * SPACE */
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static PyObject *py_unicode_strip(PyObject *self) {
  Py_ssize_t size = PyUnicode_GET_LENGTH(self);
  Py_ssize_t start, stop;
  if (py_unicode_is_ascii(self)) {
    const Py_UCS1 *data = PyUnicode_1BYTE_DATA(self);
    start = 0;
    while (start < size && ascii_whitespaces[data[start]]) start++;
    stop = size - 1;
    while (stop >= start && ascii_whitespaces[data[stop]]) stop--;
    stop++;
  } else {
    int kind = PyUnicode_KIND(self);
    const void *data = PyUnicode_DATA(self);
    start = 0;
    while (start < size &&
           Py_UNICODE_ISSPACE(PyUnicode_READ(kind, data, start)))
      start++;
    stop = size - 1;
    while (stop >= start &&
           Py_UNICODE_ISSPACE(PyUnicode_READ(kind, data, stop)))
      stop--;
    stop++;
  }
  return PyUnicode_Substring(self, start, stop);
}

static int is_sign_character(Py_UCS4 character) {
  return character == '+' || character == '-';
}

#if PY3_11_OR_MORE
static int is_delimiter(Py_UCS4 character) { return character == '_'; }

static Py_ssize_t search_unsigned_py_long(int kind, const void *data,
                                          Py_ssize_t size, Py_ssize_t start) {
  Py_ssize_t index = start;
  for (int follows_delimiter = 1; index < size; ++index) {
    Py_UCS4 character = PyUnicode_READ(kind, data, index);
    if (Py_UNICODE_ISDIGIT(character)) {
      follows_delimiter = 0;
      continue;
    } else if (!follows_delimiter && is_delimiter(character)) {
      follows_delimiter = !follows_delimiter;
      continue;
    }
    break;
  }
  return index;
}
#else
static Py_ssize_t search_unsigned_py_long(int kind, const void *data,
                                          Py_ssize_t size, Py_ssize_t start) {
  Py_ssize_t index = start;
  for (; index < size && Py_UNICODE_ISDIGIT(PyUnicode_READ(kind, data, index));
       ++index);
  return index;
}
#endif

static Py_ssize_t search_signed_PyLong(int kind, const void *data,
                                       Py_ssize_t size, Py_ssize_t start) {
  return search_unsigned_py_long(
      kind, data, size,
      start + is_sign_character(PyUnicode_READ(kind, data, start)));
}

static PyObject *parse_PyLong(PyObject *self, Py_ssize_t start,
                              Py_ssize_t stop) {
  PyObject *result_unicode = PyUnicode_Substring(self, start, stop);
  if (result_unicode == NULL) return NULL;
  PyObject *result = PyLong_FromUnicodeObject(result_unicode, 10);
  Py_DECREF(result_unicode);
  return result;
}

static PyObject *append_zeros(PyObject *self, PyObject *exponent) {
  PyObject *base = PyLong_FromLong(10);
  if (base == NULL) return NULL;
  PyObject *scale = PyNumber_Power(base, exponent, Py_None);
  Py_DECREF(base);
  if (scale == NULL) return NULL;
  PyObject *result = PyNumber_Multiply(self, scale);
  Py_DECREF(scale);
  return result;
}

static int parse_fraction_components_from_PyUnicode(
    PyObject *value, PyObject **result_numerator,
    PyObject **result_denominator) {
  Py_ssize_t size = PyUnicode_GET_LENGTH(value);
  int kind = PyUnicode_KIND(value);
  const void *data = PyUnicode_DATA(value);
  Py_UCS4 first_character = PyUnicode_READ(kind, data, 0);
  Py_ssize_t start = is_sign_character(first_character);
  Py_ssize_t numerator_stop = search_unsigned_py_long(kind, data, size, start);
  if (numerator_stop == size) {
    *result_numerator = PyLong_FromUnicodeObject(value, 10);
    if (*result_numerator == NULL) return -1;
    *result_denominator = PyLong_FromLong(1);
    if (*result_denominator == NULL) {
      Py_DECREF(*result_numerator);
      return -1;
    }
    return 0;
  }
  int is_negative = first_character == '-';
  int has_numerator = numerator_stop != start;
  *result_numerator = has_numerator ? parse_PyLong(value, start, numerator_stop)
                                    : PyLong_FromLong(0);
  if (*result_numerator == NULL) return -1;
  *result_denominator = PyLong_FromLong(1);
  if (*result_denominator == NULL) {
    Py_DECREF(*result_numerator);
    return -1;
  }
  Py_UCS4 character = PyUnicode_READ(kind, data, numerator_stop);
  if (character == '/' && start < numerator_stop && numerator_stop < size - 1) {
    Py_ssize_t denominator_stop =
        search_unsigned_py_long(kind, data, size, numerator_stop + 1);
    if (denominator_stop == size) {
      *result_numerator = parse_PyLong(value, start, numerator_stop);
      if (*result_numerator == NULL) {
        Py_DECREF(*result_denominator);
        return -1;
      }
      *result_denominator =
          parse_PyLong(value, numerator_stop + 1, denominator_stop);
      if (*result_denominator == NULL) {
        Py_DECREF(*result_numerator);
        return -1;
      }
      if (PyObject_Not(*result_denominator)) {
        PyErr_Format(PyExc_ZeroDivisionError, "Fraction(%S, 0)",
                     *result_numerator);
        Py_DECREF(*result_denominator);
        Py_DECREF(*result_numerator);
        return -1;
      }
      if (is_negative) {
        PyObject *tmp = *result_numerator;
        *result_numerator = PyNumber_Negative(*result_numerator);
        Py_DECREF(tmp);
        if (*result_numerator == NULL) {
          Py_DECREF(*result_denominator);
          return -1;
        }
      }
      return normalize_fraction_components_moduli(result_numerator,
                                                  result_denominator);
    }
  } else if (character == '.') {
    Py_ssize_t decimal_part_stop =
        search_unsigned_py_long(kind, data, size, numerator_stop + 1);
    int has_decimal_part = decimal_part_stop != numerator_stop + 1;
    if (has_decimal_part) {
      PyObject *decimal_part =
          parse_PyLong(value, numerator_stop + 1, decimal_part_stop);
      if (decimal_part == NULL) {
        Py_DECREF(*result_denominator);
        Py_DECREF(*result_numerator);
        return -1;
      }
#if PY3_11_OR_MORE
      Py_ssize_t delimiters_count = 0;
      for (Py_ssize_t index = numerator_stop + 2; index < decimal_part_stop;
           ++index)
        if (is_delimiter(PyUnicode_READ(kind, data, index))) ++delimiters_count;
      PyObject *exponent = PyLong_FromSsize_t(
          decimal_part_stop - numerator_stop - 1 - delimiters_count);
#else
      PyObject *exponent =
          PyLong_FromSsize_t(decimal_part_stop - numerator_stop - 1);
#endif
      if (exponent == NULL) {
        Py_DECREF(decimal_part);
        Py_DECREF(*result_denominator);
        Py_DECREF(*result_numerator);
        return -1;
      }
      PyObject *tmp = *result_numerator;
      *result_numerator = append_zeros(*result_numerator, exponent);
      Py_DECREF(tmp);
      if (*result_numerator == NULL) {
        Py_DECREF(exponent);
        Py_DECREF(decimal_part);
        Py_DECREF(*result_denominator);
        return -1;
      }
      tmp = *result_numerator;
      *result_numerator = PyNumber_Add(*result_numerator, decimal_part);
      Py_DECREF(tmp);
      Py_DECREF(decimal_part);
      if (*result_numerator == NULL) {
        Py_DECREF(exponent);
        Py_DECREF(*result_denominator);
        return -1;
      }
      tmp = *result_denominator;
      *result_denominator = append_zeros(*result_denominator, exponent);
      Py_DECREF(tmp);
      Py_DECREF(exponent);
      if (*result_denominator == NULL) {
        Py_DECREF(*result_numerator);
        return -1;
      }
    }
    if (decimal_part_stop == size) {
      if (is_negative) {
        PyObject *tmp = *result_numerator;
        *result_numerator = PyNumber_Negative(*result_numerator);
        Py_DECREF(tmp);
        if (*result_numerator == NULL) {
          Py_DECREF(*result_denominator);
          return -1;
        }
      }
      return normalize_fraction_components_moduli(result_numerator,
                                                  result_denominator);
    } else {
      character = PyUnicode_READ(kind, data, decimal_part_stop);
      if ((has_numerator || has_decimal_part) &&
          (character == 'e' || character == 'E')) {
        if (is_negative) {
          PyObject *tmp = *result_numerator;
          *result_numerator = PyNumber_Negative(*result_numerator);
          Py_DECREF(tmp);
          if (*result_numerator == NULL) {
            Py_DECREF(*result_denominator);
            return -1;
          }
        }
        Py_ssize_t exponent_stop =
            search_signed_PyLong(kind, data, size, decimal_part_stop + 1);
        if (exponent_stop == size) {
          PyObject *exponent =
              parse_PyLong(value, decimal_part_stop + 1, exponent_stop);
          if (exponent == NULL) {
            Py_DECREF(*result_denominator);
            Py_DECREF(*result_numerator);
            return -1;
          }
          int is_exponent_negative = is_negative_py_object(exponent);
          if (is_exponent_negative < 0) {
            Py_DECREF(exponent);
            Py_DECREF(*result_denominator);
            Py_DECREF(*result_numerator);
            return -1;
          } else if (is_exponent_negative) {
            PyObject *tmp = exponent;
            exponent = PyNumber_Negative(exponent);
            Py_DECREF(tmp);
            if (exponent == NULL) {
              Py_DECREF(*result_denominator);
              Py_DECREF(*result_numerator);
              return -1;
            }
            tmp = *result_denominator;
            *result_denominator = append_zeros(*result_denominator, exponent);
            Py_DECREF(tmp);
            Py_DECREF(exponent);
            if (*result_denominator == NULL) {
              Py_DECREF(*result_numerator);
              return -1;
            }
            return normalize_fraction_components_moduli(result_numerator,
                                                        result_denominator);
          } else {
            PyObject *tmp = *result_numerator;
            *result_numerator = append_zeros(*result_numerator, exponent);
            Py_DECREF(tmp);
            Py_DECREF(exponent);
            if (*result_numerator == NULL) {
              Py_DECREF(*result_denominator);
              return -1;
            }
            return normalize_fraction_components_moduli(result_numerator,
                                                        result_denominator);
          }
        }
      }
    }
  } else if (has_numerator && (character == 'e' || character == 'E')) {
    if (is_negative) {
      PyObject *tmp = *result_numerator;
      *result_numerator = PyNumber_Negative(*result_numerator);
      Py_DECREF(tmp);
      if (*result_numerator == NULL) {
        Py_DECREF(*result_denominator);
        return -1;
      }
    }
    Py_ssize_t exponent_stop =
        search_signed_PyLong(kind, data, size, numerator_stop + 1);
    if (exponent_stop == size) {
      PyObject *exponent =
          parse_PyLong(value, numerator_stop + 1, exponent_stop);
      if (exponent == NULL) {
        Py_DECREF(*result_denominator);
        Py_DECREF(*result_numerator);
        return -1;
      }
      int is_exponent_negative = is_negative_py_object(exponent);
      if (is_exponent_negative < 0) {
        Py_DECREF(exponent);
        Py_DECREF(*result_denominator);
        Py_DECREF(*result_numerator);
        return -1;
      } else if (is_exponent_negative) {
        PyObject *tmp = exponent;
        exponent = PyNumber_Negative(exponent);
        Py_DECREF(tmp);
        if (exponent == NULL) {
          Py_DECREF(*result_denominator);
          Py_DECREF(*result_numerator);
          return -1;
        }
        tmp = *result_denominator;
        *result_denominator = append_zeros(*result_denominator, exponent);
        Py_DECREF(tmp);
        Py_DECREF(exponent);
        if (*result_denominator == NULL) {
          Py_DECREF(*result_numerator);
          return -1;
        }
        return normalize_fraction_components_moduli(result_numerator,
                                                    result_denominator);
      } else {
        PyObject *tmp = *result_numerator;
        *result_numerator = append_zeros(*result_numerator, exponent);
        Py_DECREF(tmp);
        Py_DECREF(exponent);
        if (*result_numerator == NULL) {
          Py_DECREF(*result_denominator);
          return -1;
        }
        return normalize_fraction_components_moduli(result_numerator,
                                                    result_denominator);
      }
    }
  }
  PyErr_Format(PyExc_ValueError, "Invalid literal for Fraction: %R", value);
  return -1;
}

static FractionObject *construct_fraction(PyTypeObject *cls,
                                          PyObject *numerator,
                                          PyObject *denominator) {
  FractionObject *result = (FractionObject *)(cls->tp_alloc(cls, 0));
  if (result) {
    result->numerator = numerator;
    result->denominator = denominator;
  } else {
    Py_DECREF(denominator);
    Py_DECREF(numerator);
  }
  return result;
}

int are_kwargs_passed(PyObject *kwargs) {
  return kwargs != NULL &&
         (!PyDict_CheckExact(kwargs) || PyDict_GET_SIZE(kwargs) != 0);
}

static PyObject *fraction_new(PyTypeObject *cls, PyObject *args,
                              PyObject *kwargs) {
  if (are_kwargs_passed(kwargs)) {
    PyErr_Format(PyExc_TypeError, "Fraction() takes no keyword arguments");
    return NULL;
  }
  PyObject *numerator = NULL, *denominator = NULL;
  if (!PyArg_ParseTuple(args, "|OO", &numerator, &denominator)) return NULL;
  if (denominator) {
    if (!PyLong_Check(numerator)) {
      PyErr_SetString(PyExc_TypeError, "Numerator should be an integer.");
      return NULL;
    }
    if (!PyLong_Check(denominator)) {
      PyErr_SetString(PyExc_TypeError, "Denominator should be an integer.");
      return NULL;
    }
    if (PyObject_Not(denominator)) {
      PyErr_SetString(PyExc_ZeroDivisionError,
                      "Denominator should be non-zero.");
      return NULL;
    }
    int is_denominator_negative = is_negative_py_object(denominator);
    if (is_denominator_negative < 0)
      return NULL;
    else if (is_denominator_negative) {
      numerator = PyNumber_Negative(numerator);
      if (numerator == NULL) return NULL;
      denominator = PyNumber_Negative(denominator);
      if (denominator == NULL) {
        Py_DECREF(numerator);
        return NULL;
      }
    } else {
      Py_INCREF(numerator);
      Py_INCREF(denominator);
    }
    if (normalize_fraction_components_moduli(&numerator, &denominator) < 0) {
      Py_DECREF(numerator);
      Py_DECREF(denominator);
      return NULL;
    }
  } else if (numerator) {
    if (PyLong_Check(numerator)) {
      denominator = PyLong_FromLong(1);
      if (denominator == NULL) return NULL;
      Py_INCREF(numerator);
    } else if (PyFloat_Check(numerator)) {
      if (parse_fraction_components_from_double(PyFloat_AS_DOUBLE(numerator),
                                                &numerator, &denominator) < 0)
        return NULL;
    } else if (PyObject_TypeCheck(numerator, &FractionType)) {
      FractionObject *fraction_numerator = (FractionObject *)numerator;
      Py_INCREF(fraction_numerator->denominator);
      denominator = fraction_numerator->denominator;
      Py_INCREF(fraction_numerator->numerator);
      numerator = fraction_numerator->numerator;
    } else if (PyObject_IsInstance(numerator, Rational)) {
      if (parse_fraction_components_from_rational(numerator, &numerator,
                                                  &denominator) < 0)
        return NULL;
    } else if (PyUnicode_Check(numerator)) {
      PyObject *stripped_unicode = py_unicode_strip(numerator);
      int flag = parse_fraction_components_from_PyUnicode(
          stripped_unicode, &numerator, &denominator);
      Py_DECREF(stripped_unicode);
      if (flag < 0) return NULL;
    } else {
      PyErr_SetString(PyExc_TypeError,
                      "Single argument should be either an integer, "
                      "a floating point, a rational number or a string "
                      "representation of a fraction.");
      return NULL;
    }
  } else {
    denominator = PyLong_FromLong(1);
    numerator = PyLong_FromLong(0);
  }
  return (PyObject *)construct_fraction(cls, numerator, denominator);
}

static PyObject *Fractions_components_richcompare(PyObject *numerator,
                                                  PyObject *denominator,
                                                  PyObject *other_numerator,
                                                  PyObject *other_denominator,
                                                  int op) {
  switch (op) {
    case Py_EQ: {
      int comparison_signal =
          PyObject_RichCompareBool(numerator, other_numerator, op);
      if (comparison_signal < 0)
        return NULL;
      else if (!comparison_signal)
        Py_RETURN_FALSE;
      return PyObject_RichCompare(denominator, other_denominator, op);
    }
    case Py_NE: {
      int comparison_signal =
          PyObject_RichCompareBool(numerator, other_numerator, op);
      if (comparison_signal < 0)
        return NULL;
      else if (comparison_signal)
        Py_RETURN_TRUE;
      return PyObject_RichCompare(denominator, other_denominator, op);
    }
    default: {
      PyObject *left = PyNumber_Multiply(numerator, other_denominator);
      if (left == NULL) return NULL;
      PyObject *right = PyNumber_Multiply(other_numerator, denominator);
      if (right == NULL) {
        Py_DECREF(left);
        return NULL;
      }
      PyObject *result = PyObject_RichCompare(left, right, op);
      Py_DECREF(left);
      Py_DECREF(right);
      return result;
    }
  }
}

static PyObject *Fractions_richcompare(FractionObject *self,
                                       FractionObject *other, int op) {
  return Fractions_components_richcompare(self->numerator, self->denominator,
                                          other->numerator, other->denominator,
                                          op);
}

static PyObject *fraction_richcompare(FractionObject *self, PyObject *other,
                                      int op) {
  if (PyObject_TypeCheck(other, &FractionType))
    return Fractions_richcompare(self, (FractionObject *)other, op);
  else if (PyLong_Check(other)) {
    if (op == Py_EQ) {
      int is_integral = is_integral_fraction(self);
      if (is_integral < 0)
        return NULL;
      else if (!is_integral)
        Py_RETURN_FALSE;
      return PyObject_RichCompare(self->numerator, other, op);
    } else if (op == Py_NE) {
      int is_integral = is_integral_fraction(self);
      if (is_integral < 0)
        return NULL;
      else if (!is_integral)
        Py_RETURN_TRUE;
      return PyObject_RichCompare(self->numerator, other, op);
    } else {
      PyObject *tmp = PyNumber_Multiply(other, self->denominator);
      if (tmp == NULL) return NULL;
      PyObject *result = PyObject_RichCompare(self->numerator, tmp, op);
      Py_DECREF(tmp);
      return result;
    }
  } else if (PyFloat_Check(other)) {
    double other_value = PyFloat_AS_DOUBLE(other);
    if (!isfinite(other_value)) switch (op) {
        case Py_EQ:
          Py_RETURN_FALSE;
        case Py_GT:
        case Py_GE:
          return PyBool_FromLong(isinf(other_value) && other_value < 0.);
        case Py_LT:
        case Py_LE:
          return PyBool_FromLong(isinf(other_value) && other_value > 0.);
        case Py_NE:
          Py_RETURN_TRUE;
        default:
          return NULL;
      }
    PyObject *other_denominator, *other_numerator;
    if (parse_fraction_components_from_double(other_value, &other_numerator,
                                              &other_denominator) < 0)
      return NULL;
    return Fractions_components_richcompare(self->numerator, self->denominator,
                                            other_numerator, other_denominator,
                                            op);
  } else if (PyObject_IsInstance(other, Rational)) {
    PyObject *other_denominator, *other_numerator;
    if (parse_fraction_components_from_rational(other, &other_numerator,
                                                &other_denominator) < 0)
      return NULL;
    return Fractions_components_richcompare(self->numerator, self->denominator,
                                            other_numerator, other_denominator,
                                            op);
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *fraction_negative(FractionObject *self) {
  PyObject *numerator = PyNumber_Negative(self->numerator);
  if (numerator == NULL) return NULL;
  Py_INCREF(self->denominator);
  PyObject *denominator = self->denominator;
  return construct_fraction(&FractionType, numerator, denominator);
}

static FractionObject *fraction_absolute(FractionObject *self) {
  PyObject *numerator = PyNumber_Absolute(self->numerator);
  if (numerator == NULL) return NULL;
  Py_INCREF(self->denominator);
  PyObject *denominator = self->denominator;
  return construct_fraction(&FractionType, numerator, denominator);
}

static PyObject *fraction_float(FractionObject *self) {
  return PyNumber_TrueDivide(self->numerator, self->denominator);
}

static FractionObject *Fractions_components_add(PyObject *numerator,
                                                PyObject *denominator,
                                                PyObject *other_numerator,
                                                PyObject *other_denominator) {
  PyObject *first_result_numerator_component =
      PyNumber_Multiply(numerator, other_denominator);
  if (first_result_numerator_component == NULL) return NULL;
  PyObject *second_result_numerator_component =
      PyNumber_Multiply(other_numerator, denominator);
  if (second_result_numerator_component == NULL) {
    Py_DECREF(first_result_numerator_component);
    return NULL;
  }
  PyObject *result_numerator = PyNumber_Add(first_result_numerator_component,
                                            second_result_numerator_component);
  Py_DECREF(second_result_numerator_component);
  Py_DECREF(first_result_numerator_component);
  if (result_numerator == NULL) return NULL;
  PyObject *result_denominator =
      PyNumber_Multiply(denominator, other_denominator);
  if (result_denominator == NULL) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  if (normalize_fraction_components_moduli(&result_numerator,
                                           &result_denominator)) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *Fractions_add(FractionObject *self,
                                     FractionObject *other) {
  return Fractions_components_add(self->numerator, self->denominator,
                                  other->numerator, other->denominator);
}

static PyObject *fraction_Float_add(FractionObject *self, PyObject *other) {
  PyObject *tmp = fraction_float(self);
  if (tmp == NULL) return NULL;
  PyObject *result = PyNumber_Add(tmp, other);
  Py_DECREF(tmp);
  return result;
}

static FractionObject *fraction_Long_add(FractionObject *self,
                                         PyObject *other) {
  PyObject *tmp = PyNumber_Multiply(other, self->denominator);
  if (tmp == NULL) return NULL;
  PyObject *result_numerator = PyNumber_Add(self->numerator, tmp);
  Py_DECREF(tmp);
  if (result_numerator == NULL) return NULL;
  Py_INCREF(self->denominator);
  PyObject *result_denominator = self->denominator;
  if (normalize_fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *fraction_Rational_add(FractionObject *self,
                                             PyObject *other) {
  PyObject *other_denominator, *other_numerator;
  if (parse_fraction_components_from_rational(other, &other_numerator,
                                              &other_denominator) < 0)
    return NULL;
  FractionObject *result = Fractions_components_add(
      self->numerator, self->denominator, other_numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(other_numerator);
  return result;
}

static PyObject *fraction_add(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_add((FractionObject *)self,
                                       (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)fraction_Long_add((FractionObject *)self, other);
    else if (PyFloat_Check(other))
      return (PyObject *)fraction_Float_add((FractionObject *)self, other);
    else if (PyObject_IsInstance(other, Rational))
      return (PyObject *)fraction_Rational_add((FractionObject *)self, other);
  } else if (PyLong_Check(self))
    return (PyObject *)fraction_Long_add((FractionObject *)other, self);
  else if (PyFloat_Check(self))
    return (PyObject *)fraction_Float_add((FractionObject *)other, self);
  else if (PyObject_IsInstance(self, Rational))
    return (PyObject *)fraction_Rational_add((FractionObject *)other, self);
  Py_RETURN_NOTIMPLEMENTED;
}

static PyObject *fraction_as_integer_ratio(FractionObject *self,
                                           PyObject *Py_UNUSED(args)) {
  return PyTuple_Pack(2, self->numerator, self->denominator);
}

static PyObject *fraction_is_integer(FractionObject *self,
                                     PyObject *Py_UNUSED(args)) {
  return is_unit_py_object(self->denominator);
}

static int fraction_bool(FractionObject *self) {
  return PyObject_IsTrue(self->numerator);
}

static PyObject *fraction_ceil_impl(FractionObject *self) {
  PyObject *tmp = PyNumber_Negative(self->numerator);
  if (tmp == NULL) return NULL;
  PyObject *result = PyNumber_FloorDivide(tmp, self->denominator);
  Py_DECREF(tmp);
  if (result == NULL) return NULL;
  tmp = result;
  result = PyNumber_Negative(result);
  Py_DECREF(tmp);
  return result;
}

static PyObject *fraction_ceil(FractionObject *self,
                               PyObject *Py_UNUSED(args)) {
  return fraction_ceil_impl(self);
}

static PyObject *fraction_copy(FractionObject *self,
                               PyObject *Py_UNUSED(args)) {
  if (Py_TYPE(self) == &FractionType) {
    Py_INCREF(self);
    return (PyObject *)self;
  } else
    return PyObject_CallFunctionObjArgs(
        (PyObject *)Py_TYPE(self), self->numerator, self->denominator, NULL);
}

static PyObject *fraction_reduce(FractionObject *self,
                                 PyObject *Py_UNUSED(args)) {
  return Py_BuildValue("O(OO)", Py_TYPE(self), self->numerator,
                       self->denominator);
}

static PyObject *fraction_floor_impl(FractionObject *self) {
  return PyNumber_FloorDivide(self->numerator, self->denominator);
}

static PyObject *fraction_floor(FractionObject *self,
                                PyObject *Py_UNUSED(args)) {
  return fraction_floor_impl(self);
}

static PyObject *Fractions_components_floor_divide(
    PyObject *numerator, PyObject *denominator, PyObject *other_numerator,
    PyObject *other_denominator) {
  PyObject *dividend = PyNumber_Multiply(numerator, other_denominator);
  if (dividend == NULL) return NULL;
  PyObject *divisor = PyNumber_Multiply(denominator, other_numerator);
  if (divisor == NULL) {
    Py_DECREF(dividend);
    return NULL;
  }
  PyObject *result = PyNumber_FloorDivide(dividend, divisor);
  Py_DECREF(dividend);
  Py_DECREF(divisor);
  return result;
}

static PyObject *Fractions_floor_divide(FractionObject *self,
                                        FractionObject *other) {
  return Fractions_components_floor_divide(
      self->numerator, self->denominator, other->numerator, other->denominator);
}

static PyObject *fraction_Long_floor_divide(FractionObject *self,
                                            PyObject *other) {
  PyObject *gcd = _PyLong_GCD(self->numerator, other);
  if (gcd == NULL) return NULL;
  PyObject *dividend = PyNumber_FloorDivide(self->numerator, gcd);
  if (dividend == NULL) {
    Py_DECREF(gcd);
    return NULL;
  }
  PyObject *other_normalized = PyNumber_FloorDivide(other, gcd);
  Py_DECREF(gcd);
  if (other_normalized == NULL) {
    Py_DECREF(dividend);
    return NULL;
  }
  PyObject *divisor = PyNumber_Multiply(self->denominator, other_normalized);
  Py_DECREF(other_normalized);
  if (divisor == NULL) {
    Py_DECREF(dividend);
    return NULL;
  }
  PyObject *result = PyNumber_FloorDivide(dividend, divisor);
  Py_DECREF(dividend);
  Py_DECREF(divisor);
  return result;
}

static PyObject *Long_fraction_floor_divide(PyObject *self,
                                            FractionObject *other) {
  PyObject *gcd = _PyLong_GCD(self, other->numerator);
  if (gcd == NULL) return NULL;
  PyObject *divisor = PyNumber_FloorDivide(other->numerator, gcd);
  if (divisor == NULL) {
    Py_DECREF(gcd);
    return NULL;
  }
  PyObject *self_normalized = PyNumber_FloorDivide(self, gcd);
  Py_DECREF(gcd);
  if (self_normalized == NULL) {
    Py_DECREF(divisor);
    return NULL;
  }
  PyObject *dividend = PyNumber_Multiply(self_normalized, other->denominator);
  Py_DECREF(self_normalized);
  if (dividend == NULL) {
    Py_DECREF(divisor);
    return NULL;
  }
  PyObject *result = PyNumber_FloorDivide(dividend, divisor);
  Py_DECREF(dividend);
  Py_DECREF(divisor);
  return result;
}

static PyObject *fraction_Rational_floor_divide(FractionObject *self,
                                                PyObject *other) {
  PyObject *other_denominator, *other_numerator;
  if (parse_fraction_components_from_rational(other, &other_numerator,
                                              &other_denominator) < 0)
    return NULL;
  PyObject *result = Fractions_components_floor_divide(
      self->numerator, self->denominator, other_numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(other_numerator);
  return result;
}

static PyObject *Rational_fraction_floor_divide(PyObject *self,
                                                FractionObject *other) {
  PyObject *denominator, *numerator;
  if (parse_fraction_components_from_rational(self, &numerator, &denominator) <
      0)
    return NULL;
  PyObject *result = Fractions_components_floor_divide(
      numerator, denominator, other->numerator, other->denominator);
  Py_DECREF(denominator);
  Py_DECREF(numerator);
  return result;
}

static PyObject *fraction_floor_divide(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return Fractions_floor_divide((FractionObject *)self,
                                    (FractionObject *)other);
    else if (PyLong_Check(other))
      return fraction_Long_floor_divide((FractionObject *)self, other);
    else if (PyFloat_Check(other)) {
      PyObject *tmp = fraction_float((FractionObject *)self);
      if (tmp == NULL) return NULL;
      PyObject *result = PyNumber_FloorDivide(tmp, other);
      Py_DECREF(tmp);
      return result;
    } else if (PyObject_IsInstance(other, Rational))
      return fraction_Rational_floor_divide((FractionObject *)self, other);
  } else if (PyLong_Check(self))
    return Long_fraction_floor_divide(self, (FractionObject *)other);
  else if (PyFloat_Check(self)) {
    PyObject *tmp = fraction_float((FractionObject *)other);
    if (tmp == NULL) return NULL;
    PyObject *result = PyNumber_FloorDivide(self, tmp);
    Py_DECREF(tmp);
    return result;
  } else if (PyObject_IsInstance(self, Rational))
    return Rational_fraction_floor_divide(self, (FractionObject *)other);
  Py_RETURN_NOTIMPLEMENTED;
}

static int Longs_divmod(PyObject *dividend, PyObject *divisor,
                        PyObject **result_quotient,
                        PyObject **result_remainder) {
  PyObject *pair = PyNumber_Divmod(dividend, divisor);
  if (pair == NULL)
    return -1;
  else if (!PyTuple_Check(pair) || PyTuple_GET_SIZE(pair) != 2) {
    PyErr_SetString(PyExc_TypeError, "divmod should return pair of integers.");
    Py_DECREF(pair);
    return -1;
  }
  PyObject *quotient = PyTuple_GET_ITEM(pair, 0);
  Py_INCREF(quotient);
  PyObject *remainder = PyTuple_GET_ITEM(pair, 1);
  Py_INCREF(remainder);
  Py_DECREF(pair);
  *result_quotient = quotient;
  *result_remainder = remainder;
  return 0;
}

static PyObject *Fractions_components_divmod(PyObject *numerator,
                                             PyObject *denominator,
                                             PyObject *other_numerator,
                                             PyObject *other_denominator) {
  PyObject *dividend = PyNumber_Multiply(numerator, other_denominator);
  if (dividend == NULL) return NULL;
  PyObject *divisor = PyNumber_Multiply(other_numerator, denominator);
  if (divisor == NULL) {
    Py_DECREF(dividend);
    return NULL;
  }
  PyObject *quotient, *remainder_numerator;
  int divmod_signal =
      Longs_divmod(dividend, divisor, &quotient, &remainder_numerator);
  Py_DECREF(divisor);
  Py_DECREF(dividend);
  if (divmod_signal < 0) return NULL;
  PyObject *remainder_denominator =
      PyNumber_Multiply(denominator, other_denominator);
  if (remainder_denominator == NULL) {
    Py_DECREF(remainder_numerator);
    Py_DECREF(quotient);
    return NULL;
  }
  if (normalize_fraction_components_moduli(&remainder_numerator,
                                           &remainder_denominator) < 0) {
    Py_DECREF(remainder_denominator);
    Py_DECREF(remainder_numerator);
    Py_DECREF(quotient);
    return NULL;
  }
  FractionObject *remainder = construct_fraction(
      &FractionType, remainder_numerator, remainder_denominator);
  if (remainder == NULL) {
    Py_DECREF(quotient);
    return NULL;
  }
  return PyTuple_Pack(2, quotient, remainder);
}

static PyObject *Fractions_divmod(FractionObject *self, FractionObject *other) {
  return Fractions_components_divmod(self->numerator, self->denominator,
                                     other->numerator, other->denominator);
}

static PyObject *fraction_Long_divmod(FractionObject *self, PyObject *other) {
  PyObject *tmp = PyNumber_Multiply(other, self->denominator);
  if (tmp == NULL) return NULL;
  PyObject *quotient, *remainder_numerator;
  int divmod_signal =
      Longs_divmod(self->numerator, tmp, &quotient, &remainder_numerator);
  if (divmod_signal < 0) return NULL;
  PyObject *remainder_denominator = self->denominator;
  Py_INCREF(remainder_denominator);
  if (normalize_fraction_components_moduli(&remainder_numerator,
                                           &remainder_denominator) < 0) {
    Py_DECREF(remainder_denominator);
    Py_DECREF(remainder_numerator);
    Py_DECREF(quotient);
    return NULL;
  }
  FractionObject *remainder = construct_fraction(
      &FractionType, remainder_numerator, remainder_denominator);
  if (remainder == NULL) {
    Py_DECREF(quotient);
    return NULL;
  }
  return PyTuple_Pack(2, quotient, remainder);
}

static PyObject *Long_fraction_divmod(PyObject *self, FractionObject *other) {
  PyObject *tmp = PyNumber_Multiply(self, other->denominator);
  if (tmp == NULL) return NULL;
  PyObject *quotient, *remainder_numerator;
  int divmod_signal =
      Longs_divmod(tmp, other->numerator, &quotient, &remainder_numerator);
  if (divmod_signal < 0) return NULL;
  PyObject *remainder_denominator = other->denominator;
  Py_INCREF(remainder_denominator);
  if (normalize_fraction_components_moduli(&remainder_numerator,
                                           &remainder_denominator) < 0) {
    Py_DECREF(remainder_denominator);
    Py_DECREF(remainder_numerator);
    Py_DECREF(quotient);
    return NULL;
  }
  FractionObject *remainder = construct_fraction(
      &FractionType, remainder_numerator, remainder_denominator);
  if (remainder == NULL) {
    Py_DECREF(quotient);
    return NULL;
  }
  return PyTuple_Pack(2, quotient, remainder);
}

static PyObject *fraction_Rational_divmod(FractionObject *self,
                                          PyObject *other) {
  PyObject *other_denominator, *other_numerator;
  if (parse_fraction_components_from_rational(other, &other_numerator,
                                              &other_denominator) < 0)
    return NULL;
  PyObject *result = Fractions_components_divmod(
      self->numerator, self->denominator, other_numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(other_numerator);
  return result;
}

static PyObject *Rational_fraction_divmod(PyObject *self,
                                          FractionObject *other) {
  PyObject *denominator, *numerator;
  if (parse_fraction_components_from_rational(self, &numerator, &denominator) <
      0)
    return NULL;
  PyObject *result = Fractions_components_divmod(
      numerator, denominator, other->numerator, other->denominator);
  Py_DECREF(denominator);
  Py_DECREF(numerator);
  return result;
}

static PyObject *fraction_divmod(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return Fractions_divmod((FractionObject *)self, (FractionObject *)other);
    else if (PyLong_Check(other))
      return fraction_Long_divmod((FractionObject *)self, other);
    else if (PyFloat_Check(other)) {
      PyObject *float_self = fraction_float((FractionObject *)self);
      if (float_self == NULL) return NULL;
      PyObject *result = PyNumber_Divmod(float_self, other);
      Py_DECREF(float_self);
      return result;
    } else if (PyObject_IsInstance(other, Rational))
      return fraction_Rational_divmod((FractionObject *)self, other);
  } else if (PyLong_Check(self))
    return Long_fraction_divmod(self, (FractionObject *)other);
  else if (PyFloat_Check(self)) {
    PyObject *float_other = fraction_float((FractionObject *)other);
    if (float_other == NULL) return NULL;
    PyObject *result = PyNumber_Divmod(self, float_other);
    Py_DECREF(float_other);
    return result;
  } else if (PyObject_IsInstance(self, Rational))
    return Rational_fraction_divmod(self, (FractionObject *)other);
  Py_RETURN_NOTIMPLEMENTED;
}

static Py_hash_t fraction_hash(FractionObject *self) {
  PyObject *hash_modulus = PyLong_FromSize_t(_PyHASH_MODULUS);
  if (hash_modulus == NULL) return -1;
  PyObject *tmp = PyLong_FromSize_t(_PyHASH_MODULUS - 2);
  if (tmp == NULL) {
    Py_DECREF(hash_modulus);
    return -1;
  }
  PyObject *inverted_denominator_hash =
      PyNumber_Power(self->denominator, tmp, hash_modulus);
  Py_DECREF(tmp);
  if (inverted_denominator_hash == NULL) {
    Py_DECREF(hash_modulus);
    return -1;
  }
  PyObject *hash_;
  if (PyObject_Not(inverted_denominator_hash)) {
    Py_DECREF(inverted_denominator_hash);
    Py_DECREF(hash_modulus);
    return _PyHASH_INF;
  } else {
    PyObject *numerator_modulus = PyNumber_Absolute(self->numerator);
    if (numerator_modulus == NULL) {
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
    if (hash_ == NULL) return -1;
  }
  int is_negative = is_negative_fraction(self);
  if (is_negative < 0)
    return -1;
  else if (is_negative) {
    tmp = hash_;
    hash_ = PyNumber_Negative(hash_);
    Py_DECREF(tmp);
  }
  Py_hash_t result = PyLong_AsSsize_t(hash_);
  Py_DECREF(hash_);
  if (PyErr_Occurred()) return -1;
  return result == -1 ? -2 : result;
}

static FractionObject *Fractions_components_multiply(
    PyObject *numerator, PyObject *denominator, PyObject *other_numerator,
    PyObject *other_denominator) {
  PyObject *gcd = _PyLong_GCD(numerator, other_denominator);
  if (gcd == NULL) return NULL;
  numerator = PyNumber_FloorDivide(numerator, gcd);
  if (numerator == NULL) {
    Py_DECREF(gcd);
    return NULL;
  }
  other_denominator = PyNumber_FloorDivide(other_denominator, gcd);
  Py_DECREF(gcd);
  if (other_denominator == NULL) {
    Py_DECREF(numerator);
    return NULL;
  }
  gcd = _PyLong_GCD(other_numerator, denominator);
  if (gcd == NULL) return NULL;
  other_numerator = PyNumber_FloorDivide(other_numerator, gcd);
  if (other_numerator == NULL) {
    Py_DECREF(gcd);
    Py_DECREF(other_denominator);
    Py_DECREF(numerator);
    return NULL;
  }
  denominator = PyNumber_FloorDivide(denominator, gcd);
  Py_DECREF(gcd);
  if (denominator == NULL) {
    Py_DECREF(other_numerator);
    Py_DECREF(other_denominator);
    Py_DECREF(numerator);
    return NULL;
  }
  PyObject *result_numerator = PyNumber_Multiply(numerator, other_numerator);
  Py_DECREF(other_numerator);
  Py_DECREF(numerator);
  if (result_numerator == NULL) {
    Py_DECREF(other_denominator);
    Py_DECREF(denominator);
    return NULL;
  }
  PyObject *result_denominator =
      PyNumber_Multiply(denominator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(denominator);
  if (result_denominator == NULL) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *Fractions_multiply(FractionObject *self,
                                          FractionObject *other) {
  return Fractions_components_multiply(self->numerator, self->denominator,
                                       other->numerator, other->denominator);
}

static PyObject *fraction_Float_multiply(FractionObject *self,
                                         PyObject *other) {
  PyObject *tmp = fraction_float(self);
  if (tmp == NULL) return NULL;
  PyObject *result = PyNumber_Multiply(tmp, other);
  Py_DECREF(tmp);
  return result;
}

static FractionObject *fraction_Long_multiply(FractionObject *self,
                                              PyObject *other) {
  PyObject *gcd = _PyLong_GCD(other, self->denominator);
  if (gcd == NULL) return NULL;
  PyObject *other_normalized = PyNumber_FloorDivide(other, gcd);
  if (other_normalized == NULL) {
    Py_DECREF(gcd);
    return NULL;
  }
  PyObject *result_denominator = PyNumber_FloorDivide(self->denominator, gcd);
  Py_DECREF(gcd);
  if (result_denominator == NULL) {
    Py_DECREF(other_normalized);
    return NULL;
  }
  PyObject *result_numerator =
      PyNumber_Multiply(self->numerator, other_normalized);
  Py_DECREF(other_normalized);
  if (result_numerator == NULL) {
    Py_DECREF(result_denominator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *fraction_Rational_multiply(FractionObject *self,
                                                  PyObject *other) {
  PyObject *other_denominator, *other_numerator;
  if (parse_fraction_components_from_rational(other, &other_numerator,
                                              &other_denominator) < 0)
    return NULL;
  FractionObject *result = Fractions_components_multiply(
      self->numerator, self->denominator, other_numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(other_numerator);
  return result;
}

static PyObject *fraction_multiply(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_multiply((FractionObject *)self,
                                            (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)fraction_Long_multiply((FractionObject *)self, other);
    else if (PyFloat_Check(other))
      return (PyObject *)fraction_Float_multiply((FractionObject *)self, other);
    else if (PyObject_IsInstance(other, Rational))
      return (PyObject *)fraction_Rational_multiply((FractionObject *)self,
                                                    other);
  } else if (PyLong_Check(self))
    return (PyObject *)fraction_Long_multiply((FractionObject *)other, self);
  else if (PyFloat_Check(self))
    return (PyObject *)fraction_Float_multiply((FractionObject *)other, self);
  else if (PyObject_IsInstance(self, Rational))
    return (PyObject *)fraction_Rational_multiply((FractionObject *)other,
                                                  self);
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *Fractions_components_remainder(
    PyObject *numerator, PyObject *denominator, PyObject *other_numerator,
    PyObject *other_denominator) {
  PyObject *dividend = PyNumber_Multiply(numerator, other_denominator);
  if (dividend == NULL) return NULL;
  PyObject *divisor = PyNumber_Multiply(other_numerator, denominator);
  if (divisor == NULL) {
    Py_DECREF(dividend);
    return NULL;
  }
  PyObject *result_numerator = PyNumber_Remainder(dividend, divisor);
  Py_DECREF(dividend);
  Py_DECREF(divisor);
  if (result_numerator == NULL) return NULL;
  PyObject *result_denominator =
      PyNumber_Multiply(denominator, other_denominator);
  if (result_denominator == NULL) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  if (normalize_fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *Fractions_remainder(FractionObject *self,
                                           FractionObject *other) {
  return Fractions_components_remainder(self->numerator, self->denominator,
                                        other->numerator, other->denominator);
}

static FractionObject *fraction_Long_remainder(FractionObject *self,
                                               PyObject *other) {
  PyObject *tmp = PyNumber_Multiply(other, self->denominator);
  if (tmp == NULL) return NULL;
  PyObject *result_numerator = PyNumber_Remainder(self->numerator, tmp);
  Py_DECREF(tmp);
  if (result_numerator == NULL) return NULL;
  Py_INCREF(self->denominator);
  PyObject *result_denominator = self->denominator;
  if (normalize_fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *Long_fraction_remainder(PyObject *self,
                                               FractionObject *other) {
  PyObject *tmp = PyNumber_Multiply(self, other->denominator);
  if (tmp == NULL) return NULL;
  PyObject *result_numerator = PyNumber_Remainder(tmp, other->numerator);
  Py_DECREF(tmp);
  if (result_numerator == NULL) return NULL;
  Py_INCREF(other->denominator);
  PyObject *result_denominator = other->denominator;
  if (normalize_fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *fraction_Rational_remainder(FractionObject *self,
                                                   PyObject *other) {
  PyObject *other_denominator, *other_numerator;
  if (parse_fraction_components_from_rational(other, &other_numerator,
                                              &other_denominator) < 0)
    return NULL;
  FractionObject *result = Fractions_components_remainder(
      self->numerator, self->denominator, other_numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(other_numerator);
  return result;
}

static FractionObject *Rational_fraction_remainder(PyObject *self,
                                                   FractionObject *other) {
  PyObject *denominator, *numerator;
  if (parse_fraction_components_from_rational(self, &numerator, &denominator) <
      0)
    return NULL;
  FractionObject *result = Fractions_components_remainder(
      numerator, denominator, other->numerator, other->denominator);
  Py_DECREF(denominator);
  Py_DECREF(numerator);
  return result;
}

static PyObject *FractionObject_remainder(FractionObject *self,
                                          PyObject *other) {
  if (PyObject_TypeCheck(other, &FractionType))
    return (PyObject *)Fractions_remainder(self, (FractionObject *)other);
  else if (PyLong_Check(other))
    return (PyObject *)fraction_Long_remainder(self, other);
  else if (PyFloat_Check(other)) {
    PyObject *tmp = fraction_float(self);
    if (tmp == NULL) return NULL;
    PyObject *result = PyNumber_Remainder(tmp, other);
    Py_DECREF(tmp);
    return result;
  } else if (PyObject_IsInstance(other, Rational))
    return (PyObject *)fraction_Rational_remainder(self, other);
  Py_RETURN_NOTIMPLEMENTED;
}

static PyObject *fraction_remainder(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType))
    return FractionObject_remainder((FractionObject *)self, other);
  else if (PyLong_Check(self))
    return (PyObject *)Long_fraction_remainder(self, (FractionObject *)other);
  else if (PyFloat_Check(self)) {
    PyObject *tmp = fraction_float((FractionObject *)other);
    if (tmp == NULL) return NULL;
    PyObject *result = PyNumber_Remainder(self, tmp);
    Py_DECREF(tmp);
    return result;
  } else if (PyObject_IsInstance(self, Rational))
    return (PyObject *)Rational_fraction_remainder(self,
                                                   (FractionObject *)other);
  Py_RETURN_NOTIMPLEMENTED;
}

static PyObject *Long_fraction_power(PyObject *self, FractionObject *exponent) {
  int comparison_signal = is_integral_fraction(exponent);
  if (comparison_signal < 0)
    return NULL;
  else if (comparison_signal) {
    comparison_signal = is_negative_fraction(exponent);
    if (comparison_signal < 0)
      return NULL;
    else if (comparison_signal) {
      if (PyObject_Not(self)) {
        PyErr_SetString(PyExc_ZeroDivisionError,
                        "Either exponent should be non-negative "
                        "or base should not be zero.");
        return NULL;
      }
      PyObject *positive_exponent = PyNumber_Negative(exponent->numerator);
      if (positive_exponent == NULL) return NULL;
      PyObject *result_denominator =
          PyNumber_Power(self, positive_exponent, Py_None);
      Py_DECREF(positive_exponent);
      if (result_denominator == NULL) return NULL;
      PyObject *result_numerator = PyLong_FromLong(1);
      if (result_numerator == NULL) {
        Py_DECREF(result_denominator);
        return NULL;
      }
      return (PyObject *)construct_fraction(&FractionType, result_numerator,
                                            result_denominator);
    } else {
      PyObject *result_numerator =
          PyNumber_Power(self, exponent->numerator, Py_None);
      if (result_numerator == NULL) return NULL;
      PyObject *result_denominator = PyLong_FromLong(1);
      if (result_denominator == NULL) {
        Py_DECREF(result_numerator);
        return NULL;
      }
      return (PyObject *)construct_fraction(&FractionType, result_numerator,
                                            result_denominator);
    }
  } else {
    PyObject *float_exponent =
        PyNumber_TrueDivide(exponent->numerator, exponent->denominator);
    if (float_exponent == NULL) return NULL;
    PyObject *result = PyNumber_Power(self, float_exponent, Py_None);
    Py_DECREF(float_exponent);
    return result;
  }
}

static PyObject *Fractions_components_positive_Long_power(PyObject *numerator,
                                                          PyObject *denominator,
                                                          PyObject *exponent) {
  int comparison_signal = is_unit_py_object_bool(denominator);
  if (comparison_signal < 0)
    return NULL;
  else if (comparison_signal) {
    PyObject *result_numerator = PyNumber_Power(numerator, exponent, Py_None);
    if (result_numerator == NULL) return NULL;
    PyObject *result_denominator = PyLong_FromLong(1);
    if (result_denominator == NULL) {
      Py_DECREF(result_numerator);
      return NULL;
    }
    return (PyObject *)construct_fraction(&FractionType, result_numerator,
                                          result_denominator);
  } else {
    PyObject *result_numerator = PyNumber_Power(numerator, exponent, Py_None);
    if (result_numerator == NULL) return NULL;
    PyObject *result_denominator =
        PyNumber_Power(denominator, exponent, Py_None);
    if (result_denominator == NULL) {
      Py_DECREF(result_numerator);
      return NULL;
    }
    return (PyObject *)construct_fraction(&FractionType, result_numerator,
                                          result_denominator);
  }
}

static PyObject *fraction_components_Long_power(PyObject *numerator,
                                                PyObject *denominator,
                                                PyObject *exponent) {
  int comparison_signal = is_negative_py_object(exponent);
  if (comparison_signal < 0)
    return NULL;
  else if (comparison_signal) {
    if (PyObject_Not(numerator)) {
      PyErr_SetString(PyExc_ZeroDivisionError,
                      "Either exponent should be non-negative "
                      "or base should not be zero.");
      return NULL;
    }
    PyObject *positive_exponent = PyNumber_Negative(exponent);
    if (positive_exponent == NULL) return NULL;
    Py_INCREF(denominator);
    PyObject *inverted_numerator = denominator;
    Py_INCREF(numerator);
    PyObject *inverted_denominator = numerator;
    if (normalize_fraction_components_signs(&inverted_numerator,
                                            &inverted_denominator) < 0) {
      Py_DECREF(positive_exponent);
      return NULL;
    }
    PyObject *result = Fractions_components_positive_Long_power(
        inverted_numerator, inverted_denominator, positive_exponent);
    Py_DECREF(inverted_denominator);
    Py_DECREF(inverted_numerator);
    Py_DECREF(positive_exponent);
    return result;
  }
  return Fractions_components_positive_Long_power(numerator, denominator,
                                                  exponent);
}

static PyObject *Float_fraction_components_power(
    PyObject *self, PyObject *exponent_numerator,
    PyObject *exponent_denominator) {
  PyObject *float_exponent =
      PyNumber_TrueDivide(exponent_numerator, exponent_denominator);
  if (float_exponent == NULL) return NULL;
  PyObject *result = PyNumber_Power(self, float_exponent, Py_None);
  Py_DECREF(float_exponent);
  return result;
}

static PyObject *Float_fraction_power(PyObject *self,
                                      FractionObject *exponent) {
  return Float_fraction_components_power(self, exponent->numerator,
                                         exponent->denominator);
}

static PyObject *Fractions_components_power(PyObject *numerator,
                                            PyObject *denominator,
                                            PyObject *exponent_numerator,
                                            PyObject *exponent_denominator) {
  int is_integral_exponent = is_unit_py_object_bool(exponent_denominator);
  if (is_integral_exponent < 0)
    return NULL;
  else if (is_integral_exponent)
    return fraction_components_Long_power(numerator, denominator,
                                          exponent_numerator);
  else {
    PyObject *float_self = PyNumber_TrueDivide(numerator, denominator);
    if (float_self == NULL) return NULL;
    PyObject *result = Float_fraction_components_power(
        float_self, exponent_numerator, exponent_denominator);
    Py_DECREF(float_self);
    return result;
  }
}

static PyObject *Fractions_power(FractionObject *self,
                                 FractionObject *exponent) {
  return Fractions_components_power(self->numerator, self->denominator,
                                    exponent->numerator, exponent->denominator);
}

static PyObject *fraction_Long_power(FractionObject *self, PyObject *exponent) {
  return fraction_components_Long_power(self->numerator, self->denominator,
                                        exponent);
}

static PyObject *fraction_Rational_power(FractionObject *self,
                                         PyObject *exponent) {
  PyObject *exponent_denominator, *exponent_numerator;
  if (parse_fraction_components_from_rational(exponent, &exponent_numerator,
                                              &exponent_denominator) < 0)
    return NULL;
  PyObject *result =
      Fractions_components_power(self->numerator, self->denominator,
                                 exponent_numerator, exponent_denominator);
  Py_DECREF(exponent_denominator);
  Py_DECREF(exponent_numerator);
  return result;
}

static PyObject *Rational_fraction_power(PyObject *self,
                                         FractionObject *exponent) {
  PyObject *denominator, *numerator;
  if (parse_fraction_components_from_rational(self, &numerator, &denominator) <
      0)
    return NULL;
  PyObject *result = Fractions_components_power(
      numerator, denominator, exponent->numerator, exponent->denominator);
  Py_DECREF(denominator);
  Py_DECREF(numerator);
  return result;
}

static PyObject *fraction_power(PyObject *self, PyObject *exponent,
                                PyObject *modulo) {
  if (modulo != Py_None) {
  } else if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(exponent, &FractionType))
      return Fractions_power((FractionObject *)self,
                             (FractionObject *)exponent);
    else if (PyLong_Check(exponent))
      return fraction_Long_power((FractionObject *)self, exponent);
    else if (PyFloat_Check(exponent)) {
      PyObject *float_self, *result;
      float_self = fraction_float((FractionObject *)self);
      result = PyNumber_Power(float_self, exponent, Py_None);
      Py_DECREF(float_self);
      return result;
    } else if (PyObject_IsInstance(exponent, Rational))
      return fraction_Rational_power((FractionObject *)self, exponent);
  } else {
    assert(PyObject_TypeCheck(exponent, &FractionType) == 1);
    if (PyLong_Check(self))
      return Long_fraction_power(self, (FractionObject *)exponent);
    else if (PyFloat_Check(self))
      return Float_fraction_power(self, (FractionObject *)exponent);
    else if (PyObject_IsInstance(self, Rational))
      return Rational_fraction_power(self, (FractionObject *)exponent);
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *Fractions_components_subtract(
    PyObject *numerator, PyObject *denominator, PyObject *other_numerator,
    PyObject *other_denominator) {
  PyObject *numerator_minuend = PyNumber_Multiply(numerator, other_denominator);
  if (numerator_minuend == NULL) return NULL;
  PyObject *numerator_subtrahend =
      PyNumber_Multiply(other_numerator, denominator);
  if (numerator_subtrahend == NULL) {
    Py_DECREF(numerator_minuend);
    return NULL;
  }
  PyObject *result_numerator =
      PyNumber_Subtract(numerator_minuend, numerator_subtrahend);
  Py_DECREF(numerator_subtrahend);
  Py_DECREF(numerator_minuend);
  if (result_numerator == NULL) return NULL;
  PyObject *result_denominator =
      PyNumber_Multiply(denominator, other_denominator);
  if (result_denominator == NULL) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  if (normalize_fraction_components_moduli(&result_numerator,
                                           &result_denominator)) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *Fractions_subtract(FractionObject *self,
                                          FractionObject *other) {
  return Fractions_components_subtract(self->numerator, self->denominator,
                                       other->numerator, other->denominator);
}

static PyObject *fraction_Float_subtract(FractionObject *self,
                                         PyObject *other) {
  PyObject *tmp = fraction_float(self);
  if (tmp == NULL) return NULL;
  PyObject *result = PyNumber_Subtract(tmp, other);
  Py_DECREF(tmp);
  return result;
}

static FractionObject *fraction_Long_subtract(FractionObject *self,
                                              PyObject *other) {
  PyObject *tmp = PyNumber_Multiply(other, self->denominator);
  if (tmp == NULL) return NULL;
  PyObject *result_numerator = PyNumber_Subtract(self->numerator, tmp);
  Py_DECREF(tmp);
  Py_INCREF(self->denominator);
  PyObject *result_denominator = self->denominator;
  if (normalize_fraction_components_moduli(&result_numerator,
                                           &result_denominator) < 0) {
    Py_DECREF(result_denominator);
    Py_DECREF(result_numerator);
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *fraction_Rational_subtract(FractionObject *self,
                                                  PyObject *other) {
  PyObject *other_denominator, *other_numerator;
  if (parse_fraction_components_from_rational(other, &other_numerator,
                                              &other_denominator) < 0)
    return NULL;
  FractionObject *result = Fractions_components_subtract(
      self->numerator, self->denominator, other_numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(other_numerator);
  return result;
}

static PyObject *fraction_subtract(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_subtract((FractionObject *)self,
                                            (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)fraction_Long_subtract((FractionObject *)self, other);
    else if (PyFloat_Check(other))
      return (PyObject *)fraction_Float_subtract((FractionObject *)self, other);
    else if (PyObject_IsInstance(other, Rational))
      return (PyObject *)fraction_Rational_subtract((FractionObject *)self,
                                                    other);
  } else if (PyLong_Check(self)) {
    FractionObject *result =
        fraction_Long_subtract((FractionObject *)other, self);
    if (result == NULL) return NULL;
    PyObject *tmp = result->numerator;
    result->numerator = PyNumber_Negative(result->numerator);
    Py_DECREF(tmp);
    return (PyObject *)result;
  } else if (PyFloat_Check(self)) {
    PyObject *tmp =
        (PyObject *)fraction_Float_subtract((FractionObject *)other, self);
    if (tmp == NULL) return NULL;
    PyObject *result = PyNumber_Negative(tmp);
    Py_DECREF(tmp);
    return result;
  } else if (PyObject_IsInstance(self, Rational)) {
    FractionObject *result =
        fraction_Rational_subtract((FractionObject *)other, self);
    if (result == NULL) return NULL;
    PyObject *tmp = result->numerator;
    result->numerator = PyNumber_Negative(result->numerator);
    Py_DECREF(tmp);
    return (PyObject *)result;
  }
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *fraction_limit_denominator_impl(
    FractionObject *self, PyObject *max_denominator) {
  PyObject *tmp = PyLong_FromLong(1);
  if (tmp == NULL) return NULL;
  int comparison_signal = PyObject_RichCompareBool(max_denominator, tmp, Py_LT);
  Py_DECREF(tmp);
  if (comparison_signal < 0)
    return NULL;
  else if (comparison_signal) {
    PyErr_SetString(PyExc_ValueError,
                    "`max_denominator` should not be less than 1.");
    return NULL;
  }
  comparison_signal =
      PyObject_RichCompareBool(self->denominator, max_denominator, Py_LE);
  if (comparison_signal < 0)
    return NULL;
  else if (comparison_signal) {
    Py_INCREF(self);
    return self;
  }
  PyObject *denominator = self->denominator, *numerator = self->numerator;
  Py_INCREF(denominator);
  Py_INCREF(numerator);
  PyObject *first_bound_numerator = PyLong_FromLong(0),
           *first_bound_denominator = PyLong_FromLong(1),
           *second_bound_numerator = PyLong_FromLong(1),
           *second_bound_denominator = PyLong_FromLong(0);
  while (1) {
    PyObject *quotient = PyNumber_FloorDivide(numerator, denominator);
    if (quotient == NULL) goto error;
    PyObject *tmp = PyNumber_Multiply(quotient, second_bound_denominator);
    if (tmp == NULL) {
      Py_DECREF(quotient);
      goto loop_error;
    }
    PyObject *candidate_denominator =
        PyNumber_Add(first_bound_denominator, tmp);
    Py_DECREF(tmp);
    if (candidate_denominator == NULL) {
      Py_DECREF(quotient);
      goto loop_error;
    }
    comparison_signal =
        PyObject_RichCompareBool(candidate_denominator, max_denominator, Py_GT);
    if (comparison_signal < 0) {
      Py_DECREF(candidate_denominator);
      Py_DECREF(quotient);
      goto loop_error;
    } else if (comparison_signal) {
      Py_DECREF(candidate_denominator);
      Py_DECREF(quotient);
      break;
    }
    tmp = PyNumber_Multiply(quotient, denominator);
    if (tmp == NULL) {
      Py_DECREF(candidate_denominator);
      Py_DECREF(quotient);
      goto loop_error;
    }
    PyObject *other_tmp = PyNumber_Subtract(numerator, tmp);
    Py_DECREF(tmp);
    if (other_tmp == NULL) {
      Py_DECREF(candidate_denominator);
      Py_DECREF(quotient);
      goto loop_error;
    }
    numerator = denominator;
    denominator = other_tmp;
    Py_DECREF(first_bound_denominator);
    first_bound_denominator = second_bound_denominator;
    second_bound_denominator = candidate_denominator;
    tmp = PyNumber_Multiply(quotient, second_bound_numerator);
    Py_DECREF(quotient);
    if (tmp == NULL) goto loop_error;
    other_tmp = PyNumber_Add(first_bound_numerator, tmp);
    Py_DECREF(tmp);
    if (other_tmp == NULL) goto loop_error;
    first_bound_numerator = second_bound_numerator;
    second_bound_numerator = other_tmp;
  }
  Py_DECREF(numerator);
  Py_DECREF(denominator);
  tmp = PyNumber_Subtract(max_denominator, first_bound_denominator);
  if (tmp == NULL) goto error;
  PyObject *scale = PyNumber_FloorDivide(tmp, second_bound_denominator);
  Py_DECREF(tmp);
  if (scale == NULL) goto error;
  tmp = PyNumber_Multiply(scale, second_bound_numerator);
  if (tmp == NULL) {
    Py_DECREF(scale);
    goto error;
  }
  PyObject *other_tmp = PyNumber_Add(first_bound_numerator, tmp);
  Py_DECREF(tmp);
  if (other_tmp == NULL) {
    Py_DECREF(scale);
    goto error;
  }
  Py_DECREF(first_bound_numerator);
  first_bound_numerator = other_tmp;
  tmp = PyNumber_Multiply(scale, second_bound_denominator);
  if (tmp == NULL) {
    Py_DECREF(scale);
    goto error;
  }
  other_tmp = PyNumber_Add(first_bound_denominator, tmp);
  Py_DECREF(tmp);
  if (other_tmp == NULL) {
    Py_DECREF(scale);
    goto error;
  }
  Py_DECREF(first_bound_denominator);
  first_bound_denominator = other_tmp;
  FractionObject *first_bound = construct_fraction(
      &FractionType, first_bound_numerator, first_bound_denominator);
  if (first_bound == NULL) {
    Py_DECREF(second_bound_denominator);
    Py_DECREF(second_bound_numerator);
    return NULL;
  };
  FractionObject *second_bound = construct_fraction(
      &FractionType, second_bound_numerator, second_bound_denominator);
  if (second_bound == NULL) {
    Py_DECREF(first_bound);
    return NULL;
  }
  FractionObject *difference = Fractions_subtract(first_bound, self);
  if (difference == NULL) {
    Py_DECREF(first_bound);
    Py_DECREF(second_bound);
    return NULL;
  }
  FractionObject *first_bound_distance_to_self = fraction_absolute(difference);
  Py_DECREF(difference);
  if (first_bound_distance_to_self == NULL) {
    Py_DECREF(first_bound);
    Py_DECREF(second_bound);
    return NULL;
  }
  difference = Fractions_subtract(second_bound, self);
  if (difference == NULL) {
    Py_DECREF(first_bound_distance_to_self);
    Py_DECREF(first_bound);
    Py_DECREF(second_bound);
    return NULL;
  }
  FractionObject *second_bound_distance_to_self = fraction_absolute(difference);
  Py_DECREF(difference);
  if (second_bound_distance_to_self == NULL) {
    Py_DECREF(first_bound_distance_to_self);
    Py_DECREF(first_bound);
    Py_DECREF(second_bound);
    return NULL;
  }
  PyObject *comparison_result = Fractions_richcompare(
      second_bound_distance_to_self, first_bound_distance_to_self, Py_LE);
  Py_DECREF(first_bound_distance_to_self);
  Py_DECREF(second_bound_distance_to_self);
  if (comparison_result == NULL) {
    Py_DECREF(first_bound);
    Py_DECREF(second_bound);
    return NULL;
  } else if (comparison_result == Py_True) {
    Py_DECREF(comparison_result);
    Py_DECREF(first_bound);
    return second_bound;
  } else {
    Py_DECREF(comparison_result);
    Py_DECREF(second_bound);
    return first_bound;
  }
loop_error:
  Py_DECREF(numerator);
  Py_DECREF(denominator);
error:
  Py_DECREF(first_bound_denominator);
  Py_DECREF(first_bound_numerator);
  Py_DECREF(second_bound_denominator);
  Py_DECREF(second_bound_numerator);
  return NULL;
}

static PyObject *fraction_limit_denominator(FractionObject *self,
                                            PyObject *args) {
  PyObject *max_denominator = NULL;
  if (!PyArg_ParseTuple(args, "|O", &max_denominator)) return NULL;
  if (max_denominator == NULL) {
    max_denominator = PyLong_FromLong(1000000);
    PyObject *result =
        (PyObject *)fraction_limit_denominator_impl(self, max_denominator);
    Py_DECREF(max_denominator);
    return result;
  } else
    return (PyObject *)fraction_limit_denominator_impl(self, max_denominator);
}

static FractionObject *Fractions_components_true_divide(
    PyObject *numerator, PyObject *denominator, PyObject *other_numerator,
    PyObject *other_denominator) {
  if (PyObject_Not(other_numerator)) {
    PyErr_Format(PyExc_ZeroDivisionError, "Fraction(%S, 0)", numerator);
    return NULL;
  }
  PyObject *gcd = _PyLong_GCD(numerator, other_numerator);
  if (gcd == NULL) return NULL;
  numerator = PyNumber_FloorDivide(numerator, gcd);
  if (numerator == NULL) {
    Py_DECREF(gcd);
    return NULL;
  }
  other_numerator = PyNumber_FloorDivide(other_numerator, gcd);
  Py_DECREF(gcd);
  if (other_numerator == NULL) {
    Py_DECREF(numerator);
    return NULL;
  }
  gcd = _PyLong_GCD(denominator, other_denominator);
  if (gcd == NULL) return NULL;
  denominator = PyNumber_FloorDivide(denominator, gcd);
  if (denominator == NULL) {
    Py_DECREF(gcd);
    Py_DECREF(other_numerator);
    Py_DECREF(numerator);
    return NULL;
  }
  other_denominator = PyNumber_FloorDivide(other_denominator, gcd);
  Py_DECREF(gcd);
  if (other_denominator == NULL) {
    Py_DECREF(denominator);
    Py_DECREF(other_numerator);
    Py_DECREF(numerator);
    return NULL;
  }
  PyObject *result_numerator = PyNumber_Multiply(numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(numerator);
  if (result_numerator == NULL) {
    Py_DECREF(other_numerator);
    Py_DECREF(denominator);
    return NULL;
  }
  PyObject *result_denominator =
      PyNumber_Multiply(denominator, other_numerator);
  Py_DECREF(other_numerator);
  Py_DECREF(denominator);
  if (result_denominator == NULL) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  if (normalize_fraction_components_signs(&result_numerator,
                                          &result_denominator) < 0) {
    Py_INCREF(result_denominator);
    Py_INCREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *Fractions_true_divide(FractionObject *self,
                                             FractionObject *other) {
  return Fractions_components_true_divide(self->numerator, self->denominator,
                                          other->numerator, other->denominator);
}

static FractionObject *fraction_Long_true_divide(FractionObject *self,
                                                 PyObject *other) {
  if (PyObject_Not(other)) {
    PyErr_Format(PyExc_ZeroDivisionError, "Fraction(%S, 0)", self->numerator);
    return NULL;
  }
  PyObject *gcd = _PyLong_GCD(self->numerator, other);
  if (gcd == NULL) return NULL;
  PyObject *result_numerator = PyNumber_FloorDivide(self->numerator, gcd);
  if (result_numerator == NULL) {
    Py_DECREF(gcd);
    return NULL;
  }
  PyObject *other_normalized = PyNumber_FloorDivide(other, gcd);
  Py_DECREF(gcd);
  if (other_normalized == NULL) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  PyObject *result_denominator =
      PyNumber_Multiply(self->denominator, other_normalized);
  Py_DECREF(other_normalized);
  if (result_denominator == NULL) {
    Py_DECREF(result_numerator);
    return NULL;
  }
  if (normalize_fraction_components_signs(&result_numerator,
                                          &result_denominator) < 0) {
    Py_INCREF(result_denominator);
    Py_INCREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *Long_fraction_true_divide(PyObject *self,
                                                 FractionObject *other) {
  if (!fraction_bool(other)) {
    PyErr_Format(PyExc_ZeroDivisionError, "Fraction(%S, 0)", self);
    return NULL;
  }
  PyObject *gcd = _PyLong_GCD(self, other->numerator);
  if (gcd == NULL) return NULL;
  PyObject *result_denominator = PyNumber_FloorDivide(other->numerator, gcd);
  if (result_denominator == NULL) {
    Py_DECREF(gcd);
    return NULL;
  }
  PyObject *self_normalized = PyNumber_FloorDivide(self, gcd);
  Py_DECREF(gcd);
  if (self_normalized == NULL) {
    Py_DECREF(result_denominator);
    return NULL;
  }
  PyObject *result_numerator =
      PyNumber_Multiply(self_normalized, other->denominator);
  Py_DECREF(self_normalized);
  if (result_numerator == NULL) {
    Py_DECREF(result_denominator);
    return NULL;
  }
  if (normalize_fraction_components_signs(&result_numerator,
                                          &result_denominator) < 0) {
    Py_INCREF(result_denominator);
    Py_INCREF(result_numerator);
    return NULL;
  }
  return construct_fraction(&FractionType, result_numerator,
                            result_denominator);
}

static FractionObject *fraction_Rational_true_divide(FractionObject *self,
                                                     PyObject *other) {
  PyObject *other_denominator, *other_numerator;
  if (parse_fraction_components_from_rational(other, &other_numerator,
                                              &other_denominator) < 0)
    return NULL;
  FractionObject *result = Fractions_components_true_divide(
      self->numerator, self->denominator, other_numerator, other_denominator);
  Py_DECREF(other_denominator);
  Py_DECREF(other_numerator);
  return result;
}

static FractionObject *Rational_fraction_true_divide(PyObject *self,
                                                     FractionObject *other) {
  PyObject *denominator, *numerator;
  if (parse_fraction_components_from_rational(self, &numerator, &denominator) <
      0)
    return NULL;
  FractionObject *result = Fractions_components_true_divide(
      numerator, denominator, other->numerator, other->denominator);
  Py_DECREF(denominator);
  Py_DECREF(numerator);
  return result;
}

static PyObject *fraction_true_divide(PyObject *self, PyObject *other) {
  if (PyObject_TypeCheck(self, &FractionType)) {
    if (PyObject_TypeCheck(other, &FractionType))
      return (PyObject *)Fractions_true_divide((FractionObject *)self,
                                               (FractionObject *)other);
    else if (PyLong_Check(other))
      return (PyObject *)fraction_Long_true_divide((FractionObject *)self,
                                                   other);
    else if (PyFloat_Check(other)) {
      PyObject *tmp = fraction_float((FractionObject *)self);
      if (tmp == NULL) return NULL;
      PyObject *result = PyNumber_TrueDivide(tmp, other);
      Py_DECREF(tmp);
      return result;
    } else if (PyObject_IsInstance(other, Rational))
      return (PyObject *)fraction_Rational_true_divide((FractionObject *)self,
                                                       other);
  } else if (PyLong_Check(self))
    return (PyObject *)Long_fraction_true_divide(self, (FractionObject *)other);
  else if (PyFloat_Check(self)) {
    PyObject *tmp = fraction_float((FractionObject *)other);
    if (tmp == NULL) return NULL;
    PyObject *result = PyNumber_TrueDivide(self, tmp);
    Py_DECREF(tmp);
    return result;
  } else if (PyObject_IsInstance(self, Rational))
    return (PyObject *)Rational_fraction_true_divide(self,
                                                     (FractionObject *)other);
  Py_RETURN_NOTIMPLEMENTED;
}

static FractionObject *fraction_positive(FractionObject *self) {
  Py_INCREF(self);
  return self;
}

static PyObject *fraction_round_plain(FractionObject *self) {
  PyObject *quotient, *remainder;
  int divmod_signal =
      Longs_divmod(self->numerator, self->denominator, &quotient, &remainder);
  if (divmod_signal < 0) return NULL;
  PyObject *scalar = PyLong_FromLong(2);
  if (scalar == NULL) {
    Py_DECREF(remainder);
    Py_DECREF(quotient);
    return NULL;
  }
  PyObject *tmp = PyNumber_Multiply(remainder, scalar);
  Py_DECREF(remainder);
  if (tmp == NULL) {
    Py_DECREF(scalar);
    Py_DECREF(quotient);
    return NULL;
  }
  int comparison_signal =
      PyObject_RichCompareBool(tmp, self->denominator, Py_LT);
  if (comparison_signal < 0) {
    Py_DECREF(tmp);
    Py_DECREF(scalar);
    Py_DECREF(quotient);
    return NULL;
  } else if (comparison_signal) {
    Py_DECREF(tmp);
    Py_DECREF(scalar);
    return quotient;
  }
  comparison_signal = PyObject_RichCompareBool(tmp, self->denominator, Py_EQ);
  Py_DECREF(tmp);
  if (comparison_signal < 0) {
    Py_DECREF(scalar);
    Py_DECREF(quotient);
    return NULL;
  } else if (comparison_signal) {
    tmp = PyNumber_Remainder(quotient, scalar);
    Py_DECREF(scalar);
    if (PyObject_Not(tmp)) {
      Py_DECREF(tmp);
      return quotient;
    }
    Py_DECREF(tmp);
  }
  Py_DECREF(scalar);
  scalar = PyLong_FromLong(1);
  if (scalar == NULL) {
    Py_DECREF(quotient);
    return NULL;
  }
  tmp = quotient;
  quotient = PyNumber_Add(quotient, scalar);
  Py_DECREF(tmp);
  Py_DECREF(scalar);
  return quotient;
}

static PyObject *fraction_round(FractionObject *self, PyObject *args) {
  PyObject *precision = NULL;
  if (!PyArg_ParseTuple(args, "|O", &precision)) return NULL;
  if (precision == NULL) return fraction_round_plain(self);
  int comparison_signal = is_negative_py_object(precision);
  if (comparison_signal < 0) return NULL;
  PyObject *result_denominator, *result_numerator;
  if (comparison_signal) {
    PyObject *tmp = PyLong_FromLong(10);
    if (tmp == NULL) return NULL;
    PyObject *positive_precision = PyNumber_Negative(precision);
    if (positive_precision == NULL) {
      Py_DECREF(tmp);
      return NULL;
    }
    PyObject *shift = PyNumber_Power(tmp, positive_precision, Py_None);
    Py_DECREF(tmp);
    if (shift == NULL) return NULL;
    tmp = PyNumber_TrueDivide((PyObject *)self, shift);
    if (tmp == NULL) {
      Py_DECREF(shift);
      return NULL;
    }
    result_numerator = round_py_object(tmp);
    Py_DECREF(tmp);
    if (result_numerator == NULL) {
      Py_DECREF(shift);
      return NULL;
    }
    tmp = result_numerator;
    result_numerator = PyNumber_Multiply(result_numerator, shift);
    Py_DECREF(tmp);
    Py_DECREF(shift);
    if (result_numerator == NULL) return NULL;
    result_denominator = PyLong_FromLong(1);
    if (result_denominator == NULL) {
      Py_DECREF(result_numerator);
      return NULL;
    }
  } else {
    PyObject *tmp = PyLong_FromLong(10);
    if (tmp == NULL) return NULL;
    result_denominator = PyNumber_Power(tmp, precision, Py_None);
    Py_DECREF(tmp);
    if (result_denominator == NULL) return NULL;
    tmp = PyNumber_Multiply((PyObject *)self, result_denominator);
    if (tmp == NULL) {
      Py_DECREF(result_denominator);
      return NULL;
    }
    result_numerator = round_py_object(tmp);
    Py_DECREF(tmp);
    if (result_numerator == NULL) {
      Py_DECREF(result_denominator);
      return NULL;
    }
    if (normalize_fraction_components_moduli(&result_numerator,
                                             &result_denominator) < 0) {
      Py_DECREF(result_numerator);
      Py_DECREF(result_denominator);
      return NULL;
    }
  }
  return (PyObject *)construct_fraction(&FractionType, result_numerator,
                                        result_denominator);
}

static PyObject *fraction_repr(FractionObject *self) {
  return PyUnicode_FromFormat("Fraction(%R, %R)", self->numerator,
                              self->denominator);
}

static PyObject *fraction_str(FractionObject *self) {
  PyObject *tmp = PyLong_FromLong(1);
  int comparison_signal =
      PyObject_RichCompareBool(self->denominator, tmp, Py_EQ);
  Py_DECREF(tmp);
  if (comparison_signal < 0)
    return NULL;
  else
    return comparison_signal ? PyUnicode_FromFormat("%S", self->numerator)
                             : PyUnicode_FromFormat("%S/%S", self->numerator,
                                                    self->denominator);
}

static PyObject *fraction_trunc(FractionObject *self,
                                PyObject *Py_UNUSED(args)) {
  int is_negative = is_negative_fraction(self);
  if (is_negative < 0)
    return NULL;
  else
    return is_negative ? fraction_ceil_impl(self) : fraction_floor_impl(self);
}

static PyMemberDef fraction_members[] = {
    {"numerator", T_OBJECT_EX, offsetof(FractionObject, numerator), READONLY,
     "Numerator of the fraction."},
    {"denominator", T_OBJECT_EX, offsetof(FractionObject, denominator),
     READONLY, "Denominator of the fraction."},
    {NULL, 0, 0, 0, NULL} /* sentinel */
};

static PyMethodDef fraction_methods[] = {
    {"as_integer_ratio", (PyCFunction)fraction_as_integer_ratio, METH_NOARGS,
     NULL},
    {"is_integer", (PyCFunction)fraction_is_integer, METH_NOARGS, NULL},
    {"limit_denominator", (PyCFunction)fraction_limit_denominator, METH_VARARGS,
     NULL},
    {"__ceil__", (PyCFunction)fraction_ceil, METH_NOARGS, NULL},
    {"__copy__", (PyCFunction)fraction_copy, METH_NOARGS, NULL},
    {"__deepcopy__", (PyCFunction)fraction_copy, METH_VARARGS, NULL},
    {"__floor__", (PyCFunction)fraction_floor, METH_NOARGS, NULL},
    {"__int__", (PyCFunction)fraction_trunc, METH_NOARGS, NULL},
    {"__reduce__", (PyCFunction)fraction_reduce, METH_NOARGS, NULL},
    {"__round__", (PyCFunction)fraction_round, METH_VARARGS, NULL},
    {"__trunc__", (PyCFunction)fraction_trunc, METH_NOARGS, NULL},
    {NULL, NULL, 0, NULL} /* sentinel */
};

static PyNumberMethods fraction_as_number = {
    .nb_absolute = (unaryfunc)fraction_absolute,
    .nb_add = fraction_add,
    .nb_bool = (inquiry)fraction_bool,
    .nb_divmod = fraction_divmod,
    .nb_float = (unaryfunc)fraction_float,
    .nb_floor_divide = fraction_floor_divide,
    .nb_multiply = fraction_multiply,
    .nb_negative = (unaryfunc)fraction_negative,
    .nb_positive = (unaryfunc)fraction_positive,
    .nb_power = fraction_power,
    .nb_remainder = fraction_remainder,
    .nb_subtract = fraction_subtract,
    .nb_true_divide = fraction_true_divide,
};

static PyTypeObject FractionType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_as_number = &fraction_as_number,
    .tp_basicsize = sizeof(FractionObject),
    .tp_dealloc = (destructor)fraction_dealloc,
    .tp_doc = PyDoc_STR("Represents rational numbers in the exact form."),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_hash = (hashfunc)fraction_hash,
    .tp_itemsize = 0,
    .tp_members = fraction_members,
    .tp_methods = fraction_methods,
    .tp_name = "cfractions.Fraction",
    .tp_new = fraction_new,
    .tp_repr = (reprfunc)fraction_repr,
    .tp_richcompare = (richcmpfunc)fraction_richcompare,
    .tp_str = (reprfunc)fraction_str,
};

static PyModuleDef _cfractions_module = {
    PyModuleDef_HEAD_INIT,
    .m_doc = PyDoc_STR("Python C API alternative to `fractions` module."),
    .m_name = "cfractions",
    .m_size = -1,
};

static int load_rational() {
  PyObject *numbers_module = PyImport_ImportModule("numbers");
  if (numbers_module == NULL) return -1;
  Rational = PyObject_GetAttrString(numbers_module, "Rational");
  Py_DECREF(numbers_module);
  return !Rational ? -1 : 0;
}

static int mark_as_rational(PyObject *python_type) {
  PyObject *register_method_name = PyUnicode_FromString("register");
  if (register_method_name == NULL) return -1;
  PyObject *tmp =
#if PY3_9_OR_MORE
      PyObject_CallMethodOneArg(Rational, register_method_name, python_type);
#else
      PyObject_CallMethodObjArgs(Rational, register_method_name, python_type,
                                 NULL)
#endif
  ;
  Py_DECREF(register_method_name);
  if (tmp == NULL) return -1;
  Py_DECREF(tmp);
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
  if (load_rational() < 0) {
    Py_DECREF(result);
    return NULL;
  }
  if (mark_as_rational((PyObject *)&FractionType) < 0) {
    Py_DECREF(Rational);
    Py_DECREF(result);
    return NULL;
  }
  return result;
}
