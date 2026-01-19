cmake_minimum_required(VERSION 3.20)
project(SHA256_Project VERSION 1.0)

# Opzioni di compilazione
add_compile_options(-Wno-deprecated-declarations -pthread)

# Trova OpenSSL
find_package(OpenSSL REQUIRED)

# Include directory
include_directories(include ${OPENSSL_INCLUDE_DIR})

# Libreria comune per SHA256
add_library(sha256_util src/sha256_utils.c)
target_link_libraries(sha256_util ${OPENSSL_LIBRARIES})

# Target: Server
add_executable(server src/server.c)
target_link_libraries(server sha256_util pthread)

# Target: Client
add_executable(client src/client.c)
target_link_libraries(client sha256_util)

# Target: Query (Per la Lode)
add_executable(cache_query src/cache_query.c)
target_link_libraries(cache_query sha256_util)