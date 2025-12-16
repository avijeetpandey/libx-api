cmake_minimum_required(VERSION 3.14)
project(book_inventory_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX libpqxx)
find_package(OpenSSL REQUIRED)

include_directories(${PQXX_INCLUDE_DIRS} include)
link_directories(${PQXX_LIBRARY_DIRS})

add_executable(book_inventory
  src/main.cpp
)

# core static library for shared implementation
add_library(core STATIC
  src/services/auth_service.cpp
  src/providers/postgres_provider.cpp
  src/controllers/book_controller.cpp
  src/controllers/docs_controller.cpp
  src/services/migration_service.cpp
  src/metrics/metrics.cpp
  src/logging/logger.cpp
)
target_include_directories(core PRIVATE ${OpenSSL_INCLUDE_DIR})
target_link_libraries(core PRIVATE ${PQXX_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto pthread nlohmann_json::nlohmann_json)

target_link_libraries(book_inventory PRIVATE core)


target_include_directories(book_inventory PRIVATE ${OpenSSL_INCLUDE_DIR})
target_link_libraries(book_inventory PRIVATE ${PQXX_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto pthread)
target_link_libraries(book_inventory PRIVATE nlohmann_json::nlohmann_json)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(unit_tests
  tests/book_model_test.cpp
  tests/book_service_test.cpp
  tests/migration_service_test.cpp
  tests/metrics_test.cpp
)
target_link_libraries(unit_tests PRIVATE core gtest_main nlohmann_json::nlohmann_json)
target_include_directories(unit_tests PRIVATE include)
add_test(NAME unit_tests COMMAND unit_tests)

add_executable(auth_unit_tests tests/auth_service_test.cpp)
target_link_libraries(auth_unit_tests PRIVATE core gtest_main nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(auth_unit_tests PRIVATE include)
add_test(NAME auth_unit_tests COMMAND auth_unit_tests)

add_executable(docs_unit_tests tests/docs_test.cpp)
target_link_libraries(docs_unit_tests PRIVATE gtest_main nlohmann_json::nlohmann_json)
target_include_directories(docs_unit_tests PRIVATE include)

