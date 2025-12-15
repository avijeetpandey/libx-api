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
  src/db.cpp
)

target_include_directories(book_inventory PRIVATE ${OpenSSL_INCLUDE_DIR})
target_link_libraries(book_inventory PRIVATE ${PQXX_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto pthread)
target_link_libraries(book_inventory PRIVATE nlohmann_json::nlohmann_json)
