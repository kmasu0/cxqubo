include(FetchContent)

set(INSTALL_GTEST OFF)

FetchContent_Declare(
  googletest

  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG v1.14.0
  )

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

macro(add_cxqubo_unittest basename)
  set(test_name cxqubo_unittest_${basename})
  cxqubo_add_executable(${test_name} ${ARGN})
  set_property(TARGET ${test_name} PROPERTY FOLDER "UnitTests")
  target_link_libraries(${test_name} PRIVATE gtest_main)

  add_test(NAME ${test_name}
    COMMAND ${test_name})
endmacro()

add_subdirectory(misc)
add_subdirectory(core)
add_subdirectory(api)
