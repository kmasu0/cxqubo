include(FetchContent)

message(CHECK_START "Fetch fmt...")
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 9.1.0
)
FetchContent_MakeAvailable(fmt)

message(CHECK_PASS "fetched")
