#include <iostream>
#include <vector>
#include <stdexcept>
#include <map>
#include <complex>
#include <array>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace cpp_library{

class CppException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct Foo {
    void f(){};

    struct Child {
        void g(){};
    };
};


struct Base {
  struct Inner{};
  std::string name;
};

struct Derived : Base {
  int count;
};


struct Outer {

  struct Inner{

    enum class NestedEnum {
      ONE=1,
      TWO
    };

    NestedEnum value;
  };

  Inner inner;
};


namespace sublibA {

enum class ConsoleForegroundColor {
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35
};

enum ConsoleBackgroundColor {
    Green = 42,
    Yellow = 43,
    Blue = 44,
    Magenta = 45
};

int add(int a, int b){
    return a + b;
}

}

}


namespace forgotten {

struct Unbound {};

enum Enum{
    ONE=1,
    TWO=2
};

}

PYBIND11_MAKE_OPAQUE(std::map<std::string, std::complex<double>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::pair<std::string, double>>);

namespace py = pybind11;

PYBIND11_MODULE(python_example, m) {
    m.doc() = R"pbdoc(
        Pybind11 example test plugin
        ----------------------------

        .. currentmodule:: python_example
    )pbdoc";

  auto pyFoo = py::class_<cpp_library::Foo>(m,"Foo");
  pyFoo
    .def(py::init<>())
    .def("f",&cpp_library::Foo::f);

  py::class_<cpp_library::Foo::Child> (pyFoo, "FooChild")
    .def(py::init<>())
    .def("g",&cpp_library::Foo::Child::g);

  auto sublibA = m.def_submodule("sublibA");
  sublibA.def("add", cpp_library::sublibA::add);

  py::enum_<cpp_library::sublibA::ConsoleForegroundColor> (sublibA, "ConsoleForegroundColor")
    .value("Green", cpp_library::sublibA::ConsoleForegroundColor::Green)
    .value("Yellow", cpp_library::sublibA::ConsoleForegroundColor::Yellow)
    .value("Blue", cpp_library::sublibA::ConsoleForegroundColor::Blue)
    .value("Magenta", cpp_library::sublibA::ConsoleForegroundColor::Magenta)
    .export_values();

  py::enum_<cpp_library::sublibA::ConsoleBackgroundColor> (sublibA, "ConsoleBackgroundColor")
    .value("Green", cpp_library::sublibA::Green)
    .value("Yellow", cpp_library::sublibA::Yellow)
    .value("Blue", cpp_library::sublibA::Blue)
    .value("Magenta", cpp_library::sublibA::Magenta)
    .export_values();

  sublibA.def("accept_defaulted_enum",
      [](const cpp_library::sublibA::ConsoleForegroundColor& color){},
      py::arg("color") = cpp_library::sublibA::ConsoleForegroundColor::Blue
  );


  auto pyOuter = py::class_<cpp_library::Outer> (m, "Outer");
  auto pyInner = py::class_<cpp_library::Outer::Inner> (pyOuter, "Inner");

  py::enum_<cpp_library::Outer::Inner::NestedEnum> (pyInner, "NestedEnum")
    .value("ONE", cpp_library::Outer::Inner::NestedEnum::ONE)
    .value("TWO", cpp_library::Outer::Inner::NestedEnum::TWO)
    ;

  py::class_<cpp_library::Base> pyBase(m, "Base");

  pyBase
    .def_readwrite("name", &cpp_library::Base::name);

  py::class_<cpp_library::Base::Inner>(pyBase, "Inner");

  py::class_<cpp_library::Derived, cpp_library::Base> (m, "Derived")
    .def_readwrite("count", &cpp_library::Derived::count);

  pyInner
    .def_readwrite("value", &cpp_library::Outer::Inner::value );

  pyOuter
    .def_readwrite("inner", &cpp_library::Outer::inner)
    .attr("linalg") = py::module::import("numpy.linalg");

  py::register_exception<cpp_library::CppException>(m, "CppException");

  m.attr("foovar") = cpp_library::Foo();

  py::list foolist;
  foolist.append(cpp_library::Foo());
  foolist.append(cpp_library::Foo());

  m.attr("foolist") = foolist;
  m.attr("none") = py::none();
  {
      py::list li;
      li.append(py::none{});
      li.append(2);
      li.append(py::dict{});
      m.attr("list_with_none") = li;
  }


  auto numeric = m.def_submodule("numeric");
  numeric.def("get_ndarray_int", []{ return py::array_t<int>{}; });
  numeric.def("get_ndarray_float64", []{ return py::array_t<double>{}; });
  numeric.def("accept_ndarray_int", [](py::array_t<int>){});
  numeric.def("accept_ndarray_float64", [](py::array_t<double>){});


//  auto eigen = m.def_submodule("eigen");
//  eigen.def("get_matrix_int", []{ return Eigen::Matrix3i{}; });
//  eigen.def("get_vector_float64", []{ return Eigen::Vector3d{}; });
//  eigen.def("accept_matrix_int", [](Eigen::Matrix3i){});
//  eigen.def("accept_vector_float64", [](Eigen::Vector3d){});

  auto opaque_types = m.def_submodule("opaque_types");

  py::bind_vector<std::vector<std::pair<std::string, double>>>(opaque_types, "VectorPairStringDouble");
  py::bind_map<std::map<std::string, std::complex<double>>>(opaque_types, "MapStringComplex");

  opaque_types.def("get_complex_map", []{return std::map<std::string, std::complex<double>>{}; });
  opaque_types.def("get_vector_of_pairs", []{return std::vector<std::pair<std::string, double>>{}; });

  auto copy_types = m.def_submodule("copy_types");
  copy_types.def("get_complex_map", []{return std::map<int, std::complex<double>>{}; });
  copy_types.def("get_vector_of_pairs", []{return std::vector<std::pair<int, double>>{}; });

  // This submodule will have C++ signatures in python docstrings to emulate poorly written pybind11-bindings
  auto invalid_signatures = m.def_submodule("invalid_signatures");
  invalid_signatures.def("get_unbound_type", []{return forgotten::Unbound{}; });
  invalid_signatures.def("accept_unbound_type", [](std::pair<forgotten::Unbound, int>){ return 0;});
  invalid_signatures.def("accept_unbound_enum", [](forgotten::Enum){ return 0;});

  py::class_<forgotten::Unbound>(invalid_signatures, "Unbound");
  py::class_<forgotten::Enum>(invalid_signatures, "Enum");
  invalid_signatures.def("accept_unbound_type_defaulted", [](forgotten::Unbound){ return 0;}, py::arg("x")=forgotten::Unbound{});
  invalid_signatures.def("accept_unbound_enum_defaulted", [](forgotten::Enum){ return 0;}, py::arg("x")=forgotten::Enum::ONE);

}
