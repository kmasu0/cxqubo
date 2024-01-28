include(FetchContent)

message(CHECK_START "Fetch cimod...")
FetchContent_Declare(cimod
  GIT_REPOSITORY https://github.com/OpenJij/cimod
  GIT_TAG v1.5.1
)
FetchContent_MakeAvailable(cimod)

message(CHECK_PASS "fetched")
