#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

namespace py = pybind11;

#define MODULE_NAME _cfractions
#define FRACTION_NAME "Fraction"
#define C_STR_HELPER(a) #a
#define C_STR(a) C_STR_HELPER(a)
#ifndef VERSION_INFO
#define VERSION_INFO "dev"
#endif

template <class Object>
std::string to_repr(const Object& object) {
  std::ostringstream stream;
  stream << object;
  return stream.str();
}

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

  std::string repr() const {
    PyObject* str_value = PyObject_Repr(ptr());
    if (!str_value) throw py::error_already_set();
    PyObject* temp = PyUnicode_AsUTF8String(str_value);
    if (!temp) throw py::error_already_set();
    char* buffer;
    ssize_t length;
    if (PyBytes_AsStringAndSize(temp, &buffer, &length))
      throw py::error_already_set();
    return std::string(buffer, static_cast<std::size_t>(length));
  }

 private:
  std::shared_ptr<PyObject> _ptr;
};

static std::ostream& operator<<(std::ostream& stream, const Object& object) {
  return stream << object.repr();
}

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
    return *this + other.object();
  }

  Int operator+(const Object& other) const {
    PyObject* result_ptr = PyNumber_Add(object().ptr(), other.ptr());
    if (!result_ptr)
      throw py::error_already_set();
    return Object(result_ptr, false);
  }

  Int operator/(const Int& other) const {
    return *this / other.object();
  }

  Int operator/(const Object& other) const {
    PyObject* result_ptr = PyNumber_FloorDivide(object().ptr(), other.ptr());
    if (!result_ptr)
      throw py::error_already_set();
    return Object(result_ptr, false);
  }

  Int operator%(const Int& other) const {
    return *this % other.object();
  }

  Int operator%(const Object& other) const {
    PyObject* result_ptr = PyNumber_Remainder(object().ptr(), other.ptr());
    if (!result_ptr)
      throw py::error_already_set();
    return Object(result_ptr, false);
  }

  Int operator*(const Int& other) const {
    return *this * other.object();
  }

  Int operator*(const Object& other) const {
    PyObject* result_ptr = PyNumber_Multiply(object().ptr(), other.ptr());
    if (!result_ptr)
      throw py::error_already_set();
    return Object(result_ptr, false);
  }

  Int operator-(const Int& other) const {
    return Object(PyNumber_Subtract(object().ptr(), other.object().ptr()),
                  false);
  }

  Int operator-(const Object& other) const {
    PyObject* result_ptr = PyNumber_Subtract(object().ptr(), other.ptr());
    if (!result_ptr)
      throw py::error_already_set();
    return Object(result_ptr, false);
  }

  Int operator-() const {
    return Object(PyNumber_Negative(object().ptr()), false);
  }

  Int& operator=(Int&& other) {
    _object = std::move(other.object());
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

  Int abs() const { return *this < 0 ? -(*this) : *this; }

 private:
  Object _object;
};

static std::ostream& operator<<(std::ostream& stream, const Int& int_) {
  return stream << int_.object();
}

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
    auto gcd = to_gcd(_numerator, _denominator);
    _numerator /= gcd;
    _denominator /= gcd;
    if (_denominator < 0) {
      _denominator = -_denominator;
      _numerator = -_numerator;
    }
  };

  Fraction abs() const { return Fraction(numerator().abs(), denominator()); }

  const Int& numerator() const { return _numerator; }

  const Int& denominator() const { return _denominator; }

  bool operator==(const Fraction& other) const {
    return numerator() == other.numerator() &&
           denominator() == other.denominator();
  }

 private:
  Int _numerator, _denominator;
};

static std::ostream& operator<<(std::ostream& stream,
                                const Fraction& fraction) {
  return stream << FRACTION_NAME << "(" << fraction.numerator() << ", "
                << fraction.denominator() << ")";
}

PYBIND11_MODULE(MODULE_NAME, m) {
  m.doc() = R"pbdoc(Python C API alternative to `fractions` module.)pbdoc";
  m.attr("__version__") = C_STR(VERSION_INFO);

  py::class_<Fraction>(m, FRACTION_NAME)
      .def(py::init(
               [](const py::handle& numerator, const py::handle& denominator) {
                 if (!PyObject_IsTrue(denominator.ptr())) {
                   PyErr_SetString(PyExc_ZeroDivisionError,
                                   "Denominator should be non-zero.");
                   throw py::error_already_set();
                 }
                 return Fraction(Object(numerator.ptr()),
                                 Object(denominator.ptr()));
               }),
           py::arg("numerator"), py::arg("denominator"))
      .def(py::self == py::self)
      .def("__abs__", &Fraction::abs)
      .def("__repr__", to_repr<Fraction>)
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
