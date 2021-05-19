#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

#define MODULE_NAME _cfractions
#define C_STR_HELPER(a) #a
#define C_STR(a) C_STR_HELPER(a)
#ifndef VERSION_INFO
#define VERSION_INFO "dev"
#endif

class Int {
 public:
  Int(PyObject* ptr) : _ptr(ptr) {}

  PyObject* ptr() const { return _ptr; }

  operator bool() const { return PyObject_IsTrue(ptr()); }

  bool operator==(const Int& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_EQ);
  }

  bool operator>=(const Int& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_GE);
  }

  bool operator>(const Int& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_GT);
  }

  bool operator<=(const Int& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_LE);
  }

  bool operator<(const Int& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_LT);
  }

  Int operator+(const Int& other) const {
    return Int(PyNumber_Add(ptr(), other.ptr()));
  }

  Int operator/(const Int& other) const {
    return Int(PyNumber_FloorDivide(ptr(), other.ptr()));
  }

  Int operator%(const Int& other) const {
    return Int(PyNumber_Remainder(ptr(), other.ptr()));
  }

  Int operator*(const Int& other) const {
    return Int(PyNumber_Multiply(ptr(), other.ptr()));
  }

  Int operator-(const Int& other) const {
    return Int(PyNumber_Subtract(ptr(), other.ptr()));
  }

  Int& operator=(const Int& other) {
    _ptr = other.ptr();
    return *this;
  }

  Int& operator+=(const Int& other) {
    return *this = *this + other;
  }

  Int& operator/=(const Int& other) {
    return *this = *this / other;
  }

  Int& operator%=(const Int& other) {
    return *this = *this % other;
  }

  Int& operator*=(const Int& other) {
    return *this = *this * other;
  }

  Int& operator-=(const Int& other) {
    return *this = *this - other;
  }

 private:
  PyObject* _ptr;
};

Int to_gcd(const Int& left, const Int& right) {
  Int result = std::max(left, right), remainder = std::min(left, right);
  while (remainder) {
    Int step = result % remainder;
    result = remainder;
    remainder = step;
  }
  return result;
}

class Fraction {
 public:
  Fraction(const Int& numerator, const Int& denominator)
      : _numerator(numerator), _denominator(denominator) {
    auto gcd = to_gcd(_numerator, _denominator);
    _numerator /= gcd;
    _denominator /= gcd;
  };

  const Int& numerator() const { return _numerator; }

  const Int& denominator() const { return _denominator; }

  bool operator==(const Fraction& other) const {
    return numerator() == other.numerator() &&
           denominator() == other.denominator();
  }

 private:
  Int _numerator, _denominator;
};

PYBIND11_MODULE(MODULE_NAME, m) {
  m.doc() = R"pbdoc(Python C API alternative to `fractions` module.)pbdoc";
  m.attr("__version__") = C_STR(VERSION_INFO);

  py::class_<Fraction>(m, "Fraction")
      .def(py::init([](const py::int_& numerator, const py::int_& denominator) {
             if (!PyObject_IsTrue(denominator.ptr()))
               throw py::value_error("Denominator should be non-zero.");
             return Fraction(numerator.ptr(), denominator.ptr());
           }),
           py::arg("numerator"), py::arg("denominator"))
      .def(py::self == py::self)
      .def_property_readonly(
          "denominator",
          [](const Fraction& self) {
            return py::reinterpret_borrow<py::int_>(self.denominator().ptr());
          })
      .def_property_readonly("numerator", [](const Fraction& self) {
        return py::reinterpret_borrow<py::int_>(self.numerator().ptr());
      });
}
