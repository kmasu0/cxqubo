add_cxqubo_unittest(core
  entity_test.cpp
  vartypes_test.cpp
  conditions_test.cpp
  context_test.cpp
  express_test.cpp
  compile_test.cpp

  LINK_CXQUBO_LIBS
    header_only
)
