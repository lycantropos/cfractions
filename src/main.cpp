#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include <memory>
#include <type_traits>

namespace py = pybind11;

#define MODULE_NAME _cfractions
#define FRACTION_NAME "Fraction"
#define C_STR_HELPER(a) #a
#define C_STR(a) C_STR_HELPER(a)
#ifndef VERSION_INFO
#define VERSION_INFO "dev"
#endif

template <typename Raw, std::enable_if_t<std::is_integral<Raw>::value, int> = 0>
static PyObject* pack_integer(Raw value) {
  PyObject* result = nullptr;
  if (sizeof(Raw) <= sizeof(long)) {
    if (std::is_signed<Raw>::value)
      result = PyLong_FromLong(static_cast<long>(value));
    else
      result = PyLong_FromUnsignedLong(static_cast<unsigned long>(value));
  } else {
    if (std::is_signed<Raw>::value)
      result = PyLong_FromLongLong(static_cast<long long>(value));
    else
      result =
          PyLong_FromUnsignedLongLong(static_cast<unsigned long long>(value));
  }
  if (!result) throw py::value_error("Could not allocate `int` object.");
  return result;
}

static void decrease_reference_count(PyObject* object) { Py_DECREF(object); }

static void increase_reference_count(PyObject* object) { Py_INCREF(object); }

class Object {
 public:
  Object() = delete;

  Object(PyObject* ptr, bool borrowed = true)
      : _ptr(ptr, &decrease_reference_count) {
    if (borrowed) increase_reference_count(ptr);
  }

  Object(const Object& other) : _ptr(other._ptr) {}

  Object(Object&& other) : _ptr(std::move(other._ptr)) {}

  Object& operator=(const Object& other) {
    _ptr = other._ptr;
    return *this;
  }

  Object& operator=(Object&& other) {
    _ptr.reset(other.ptr(), &decrease_reference_count);
    return *this;
  }

  PyObject* ptr() const { return _ptr.get(); }

  operator bool() const { return PyObject_IsTrue(ptr()); }

  bool operator==(const Object& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_EQ);
  }

  bool operator>=(const Object& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_GE);
  }

  bool operator>(const Object& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_GT);
  }

  bool operator<=(const Object& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_LE);
  }

  bool operator<(const Object& other) const {
    return PyObject_RichCompareBool(ptr(), other.ptr(), Py_LT);
  }

 private:
  std::shared_ptr<PyObject> _ptr;
};

class Int {
 public:
  Int() = delete;

  Int(const Object& object) : _object(object) {}

  Int(const Int& other) : _object(other.object()) {}

  Int(Int&& other) : _object(std::move(other.object())) {}

  template <class Raw, std::enable_if_t<std::is_integral<Raw>::value, int> = 0>
  Int(Raw raw) : _object(pack_integer(raw), false) {}

  const Object& object() const { return _object; }

  operator bool() const { return bool(object()); }

  bool operator==(const Int& other) const { return object() == other.object(); }

  bool operator>=(const Int& other) const { return object() >= other.object(); }

  bool operator>(const Int& other) const { return object() > other.object(); }

  bool operator<=(const Int& other) const { return object() <= other.object(); }

  bool operator<(const Int& other) const { return object() < other.object(); }

  template <class Raw>
  bool operator==(Raw value) const {
    return *this == Int(value);
  }

  template <class Raw>
  bool operator<(Raw value) const {
    return *this < Int(value);
  }

  template <class Raw>
  bool operator>(Raw value) const {
    return *this > Int(value);
  }

  template <class Raw>
  bool operator<=(Raw value) const {
    return *this <= Int(value);
  }

  template <class Raw>
  bool operator>=(Raw value) const {
    return *this >= Int(value);
  }

  Int operator+(const Int& other) const {
    return Object(PyNumber_Add(object().ptr(), other.object().ptr()), false);
  }

  Int operator/(const Int& other) const {
    return Object(PyNumber_FloorDivide(object().ptr(), other.object().ptr()),
                  false);
  }

  Int operator%(const Int& other) const {
    return Object(PyNumber_Remainder(object().ptr(), other.object().ptr()),
                  false);
  }

  Int operator*(const Int& other) const {
    return Object(PyNumber_Multiply(object().ptr(), other.object().ptr()),
                  false);
  }

  Int operator-(const Int& other) const {
    return Object(PyNumber_Subtract(object().ptr(), other.object().ptr()),
                  false);
  }

  Int operator-() const {
    return Object(PyNumber_Negative(object().ptr()), false);
  }

  Int& operator=(Int&& other) {
    _object = other.object();
    return *this;
  }

  Int& operator=(const Int& other) {
    _object = other.object();
    return *this;
  }

  Int& operator+=(const Int& other) { return *this = *this + other; }

  Int& operator/=(const Int& other) { return *this = *this / other; }

  Int& operator%=(const Int& other) { return *this = *this % other; }

  Int& operator*=(const Int& other) { return *this = *this * other; }

  Int& operator-=(const Int& other) { return *this = *this - other; }

 private:
  Object _object;
};

static Int to_gcd(const Int& left, const Int& right) {
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
    if (_denominator < 0) {
      _denominator = -_denominator;
      _numerator = -_numerator;
    }
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

  py::class_<Fraction>(m, FRACTION_NAME)
      .def(py::init(
               [](const py::handle& numerator, const py::handle& denominator) {
                 if (!PyObject_IsTrue(denominator.ptr()))
                   throw py::value_error("Denominator should be non-zero.");
                 return Fraction(Object(numerator.ptr()),
                                 Object(denominator.ptr()));
               }),
           py::arg("numerator"), py::arg("denominator"))
      .def(py::self == py::self)
      .def_property_readonly("denominator",
                             [](const Fraction& self) {
                               return py::reinterpret_borrow<py::int_>(
                                   self.denominator().object().ptr());
                             })
      .def_property_readonly("numerator", [](const Fraction& self) {
        return py::reinterpret_borrow<py::int_>(
            self.numerator().object().ptr());
      });
}
