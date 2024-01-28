include(GNUInstallDirs)

set(CXQUBO_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CXQUBO_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CXQUBO_INCLUDE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include)
set(CXQUBO_SOURCE_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CXQUBO_VERSION_SUFFIX "git" CACHE STRING "version suffix")
set(CXQUBO_MODULE_PREFIX cxqubo_)

function(cxqubo_add_target name)
  install(TARGETS ${name}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${name}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${name}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${name}
    )

  set_target_properties(${name} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CXQUBO_LIBRARY_OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${CXQUBO_LIBRARY_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CXQUBO_RUNTIME_OUTPUT_DIRECTORY}"
    )
endfunction(cxqubo_add_target)

function(cxqubo_add_scripts name)
  foreach(script ${ARGN})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${script}
      TYPE BIN
      )
  endforeach()
endfunction(cxqubo_add_scripts)

function(cxqubo_set_rpath name)
  if (NOT CMAKE_INSTALL_RPATH_USE_LINK_PATH)
    set_target_properties(${name} PROPERTIES
      INSTALL_RPATH "\$ORIGIN/../lib"
      )
  endif()
endfunction(cxqubo_set_rpath)

function(cxqubo_add_header_library basename)
  cmake_parse_arguments(ARG
    ""
    ""
    "LINK_LIBS;LINK_CXQUBO_LIBS"
    ${ARGN})

  set(name ${CXQUBO_MODULE_PREFIX}${basename})
  add_library(${name} INTERFACE)

  set(link_libs ${ARG_LINK_LIBS})
  foreach(libname ${ARG_LINK_CXQUBO_LIBS})
    list(APPEND link_libs ${CXQUBO_MODULE_PREFIX}${libname})
  endforeach()

  target_include_directories(${name} INTERFACE
    $<BUILD_INTERFACE:${CXQUBO_INCLUDE_DIRECTORY}>
    $<INSTALL_INTERFACE:include>
  )
  target_include_directories(${name} INTERFACE
    $<BUILD_INTERFACE:${CXQUBO_SOURCE_INCLUDE_DIRECTORY}>
    $<INSTALL_INTERFACE:include>
  )
  target_link_libraries(${name} INTERFACE ${link_libs})

  cxqubo_add_target(${name})
  set_property(GLOBAL APPEND PROPERTY CXQUBO_ALL_LIBS ${name})
endfunction(cxqubo_add_header_library)

function(cxqubo_add_library basename)
  cmake_parse_arguments(ARG
    "STATIC;SHARED"
    ""
    "LINK_LIBS;LINK_CXQUBO_LIBS"
    ${ARGN})

  set(name ${CXQUBO_MODULE_PREFIX}${basename})
  set(link_type PRIVATE)
  if (ARG_STATIC OR NOT BUILD_SHARED_LIBS)
    set(link_type INTERFACE)
    add_library(${name} STATIC ${ARG_UNPARSED_ARGUMENTS})
  else()
    add_library(${name} SHARED ${ARG_UNPARSED_ARGUMENTS})
    cxqubo_set_rpath(${name})
    set_target_properties(${name} PROPERTIES
      SOVERSION ${CXQUBO_VERSION_MAJOR}${CXQUBO_VERSION_SUFFIX}
      VERSION ${CXQUBO_VERSION_MAJOR}${CXQUBO_VERSION_SUFFIX})
  endif()

  set(link_libs ${ARG_LINK_LIBS})
  foreach(libname ${ARG_LINK_CXQUBO_LIBS})
    list(APPEND link_libs ${CXQUBO_MODULE_PREFIX}${libname})
  endforeach()

  target_link_libraries(${name} ${link_type} ${link_libs})

  cxqubo_add_target(${name})
  set_property(GLOBAL APPEND PROPERTY CXQUBO_ALL_LIBS ${name})
endfunction(cxqubo_add_library)

function(cxqubo_add_executable name)
  cmake_parse_arguments(ARG
    ""
    ""
    "LINK_LIBS;LINK_CXQUBO_LIBS"
    ${ARGN})

  add_executable(${name} ${ARG_UNPARSED_ARGUMENTS})
  cxqubo_set_rpath(${name})

  set(link_libs ${ARG_LINK_LIBS})
  foreach(libname ${ARG_LINK_CXQUBO_LIBS})
    list(APPEND link_libs ${CXQUBO_MODULE_PREFIX}${libname})
  endforeach()

  target_link_libraries(${name} PRIVATE ${link_libs})

  set_target_properties(${name} PROPERTIES ENABLE_EXPORTS 1)
  cxqubo_add_target(${name})
endfunction(cxqubo_add_executable)

