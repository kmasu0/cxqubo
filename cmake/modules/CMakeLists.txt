include(GNUInstallDirs)

install(DIRECTORY .
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cxqubo
  COMPONENT cmake-exports
  FILES_MATCHING PATTERN *.cmake
)
