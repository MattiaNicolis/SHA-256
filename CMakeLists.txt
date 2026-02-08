cmake_minimum_required(VERSION 3.20)
project(SHA256_Project VERSION 1.0)

add_compile_options(-Wno-deprecated-declarations -pthread)

find_package(OpenSSL REQUIRED)

include_directories(include ${OPENSSL_INCLUDE_DIR})

add_library(sha256_util src/sha256_utils.c)
target_link_libraries(sha256_util ${OPENSSL_LIBRARIES})

add_executable(server src/server.c)
target_link_libraries(server sha256_util pthread)

add_executable(client src/client.c)
target_link_libraries(client sha256_util)

add_executable(cache_query src/cache_query.c)
target_link_libraries(cache_query sha256_util)