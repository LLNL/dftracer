include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/dftracer-utils.cmake)
install_external_project(cpp-logger 1.0.0 "CPP_LOGGER" https://github.com/hariharan-devarajan/cpp-logger.git v0.0.4 ${CMAKE_INSTALL_PREFIX} "")
install_external_project(gotcha 2.3.2 "gotcha" https://github.com/LLNL/GOTCHA.git 1.0.7 ${CMAKE_INSTALL_PREFIX} "")
install_external_project(brahma 2.1.0 "BRAHMA" https://github.com/hariharan-devarajan/brahma.git v0.0.9 ${CMAKE_INSTALL_PREFIX} "")
add_dependencies(brahma cpp-logger)
add_dependencies(brahma gotcha)
find_package(yaml-cpp QUIET)
install_external_project(yaml-cpp "" "YAML_CPP" https://github.com/jbeder/yaml-cpp.git yaml-cpp-0.6.3 ${CMAKE_INSTALL_PREFIX} "-DYAML_CPP_BUILD_TESTS=OFF;-DYAML_BUILD_SHARED_LIBS=ON")
if (NOT ${yaml-cpp_FOUND})
  add_dependencies(yaml-cpp brahma)
  add_dependencies(yaml-cpp gotcha)
  add_dependencies(yaml-cpp cpp-logger)
endif()
install_external_project(pybind11 2.12.0 "pybind11" https://github.com/pybind/pybind11.git v2.12.0 ${CMAKE_INSTALL_PREFIX} "-DPYBIND11_TEST=OFF")
