set(COMMON_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wwrite-strings")
set(COMMON_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wmissing-field-initializers -Wimplicit-fallthrough -Werror -Wno-uninitialized -Wno-defaulted-function-deleted")
# set(COMMON_CXX_FLAGS "-stdlib=libc++ -Wall -Wextra -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wmissing-field-initializers -Wimplicit-fallthrough -Werror -Wno-uninitialized -Wno-defaulted-function-deleted")
set(COMMON_FLAGS_RELEASE "-Ofast -DNDEBUG=1")
set(COMMON_FLAGS_RELWITHDEBUGINFO "-Ofast -DNDEBUG=1 -g3 -gdwarf-4")
set(COMMON_FLAGS_DEBUG "-O0 -g3 -gdwarf-4")
# set(COMMON_FLAGS_DEBUG "-O0 -g3 -gdwarf-4 -fsanitize=memory")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${COMMON_C_FLAGS} ${COMMON_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELWITHDEBUGINFO "${COMMON_C_FLAGS} ${COMMON_FLAGS_RELWITHDEBUGINFO}")
set(CMAKE_C_FLAGS_RELEASE "${COMMON_C_FLAGS} ${COMMON_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_CXX_FLAGS} ${COMMON_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELWITHDEBUGINFO "${COMMON_CXX_FLAGS} ${COMMON_FLAGS_RELWITHDEBUGINFO}")
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_CXX_FLAGS} ${COMMON_FLAGS_RELEASE}")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -fsanitize=memory")
