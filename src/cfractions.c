#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include <structmember.h>

typedef struct {
  PyObject_HEAD PyObject *numerator;
  PyObject *denominator;
} FractionObject;

static void Fraction_dealloc(FractionObject *self) {
  Py_XDECREF(self->numerator);
  Py_XDECREF(self->denominator);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *Fraction_new(PyTypeObject *type, PyObject *args,
                              PyObject *kwargs) {
  FractionObject *self;
  self = (FractionObject *)type->tp_alloc(type, 0);
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
      denominator = PyNumber_Negative(denominator);
      numerator = PyNumber_Negative(numerator);
    } else {
      Py_INCREF(numerator);
      Py_INCREF(denominator);
    }
    Py_DECREF(tmp);

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

static PyObject *Fraction_repr(FractionObject *self) {
  return PyUnicode_FromFormat("Fraction(%R, %R)", self->numerator,
                              self->denominator);
}

static PyTypeObject FractionType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "_cfractions.Fraction",
    .tp_doc = "Represents rational numbers in the exact form.",
    .tp_basicsize = sizeof(FractionObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Fraction_new,
    .tp_init = (initproc)Fraction_init,
    .tp_dealloc = (destructor)Fraction_dealloc,
    .tp_members = Fraction_members,
    .tp_repr = (reprfunc)Fraction_repr,
};

static PyModuleDef _cfractions_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_cfractions",
    .m_doc = "Python C API alternative to `fractions` module.",
    .m_size = -1,
};

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
  return result;
}
