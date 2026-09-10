cmake_minimum_required(VERSION 3.20)

# Prefer Clang when the user has not selected a compiler explicitly:
# cmake -S . -B build
# Users can still override this with -DCMAKE_CXX_COMPILER=/path/to/compiler.

if(NOT DEFINED CMAKE_CXX_COMPILER AND NOT CMAKE_CXX_COMPILER)
    find_program(PASSMAN_CLANGXX clang++)

    if(PASSMAN_CLANGXX)
        set(
            CMAKE_CXX_COMPILER
            "${PASSMAN_CLANGXX}"
            CACHE FILEPATH
            "C++ compiler used to build passman"
            FORCE
        )
    endif()
endif()

project(passman VERSION 1.0.0 LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    PASSMAN_ENABLE_SANITIZERS
    "Enable AddressSanitizer and UndefinedBehaviorSanitizer"
    OFF
)


# Source files


add_executable(passman
    src/cli.cpp
    src/clipboard.cpp
    src/clipboard_timer.cpp
    src/commands.cpp
    src/crypto.cpp
    src/entry.cpp
    src/export.cpp
    src/main.cpp
    src/password.cpp
    src/session.cpp
    src/vault.cpp
)


# Include directories


target_include_directories(passman PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)


# Compiler warnings

target_compile_options(passman PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
)

# libsodium
find_path(
    SODIUM_INCLUDE_DIR
    sodium.h
    PATHS
        /opt/homebrew/include
        /usr/local/include
)

find_library(
    SODIUM_LIBRARY
    sodium
    PATHS
        /opt/homebrew/lib
        /usr/local/lib
)

if(NOT SODIUM_INCLUDE_DIR OR NOT SODIUM_LIBRARY)
    message(FATAL_ERROR
        "libsodium not found. Install it with: brew install libsodium"
    )
endif()

target_include_directories(passman PRIVATE
    "${SODIUM_INCLUDE_DIR}"
)

target_link_libraries(passman PRIVATE
    "${SODIUM_LIBRARY}"
)

include(GNUInstallDirs)
install(TARGETS passman
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if(BUILD_TESTING)
    add_test(NAME passman.cli.help COMMAND passman --help)

    add_executable(passman_crypto_test
        tests/crypto_test.cpp
        src/crypto.cpp
    )

    target_include_directories(passman_crypto_test PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        "${SODIUM_INCLUDE_DIR}"
    )

    target_link_libraries(passman_crypto_test PRIVATE
        "${SODIUM_LIBRARY}"
    )

    target_compile_options(passman_crypto_test PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )

    add_test(NAME passman.crypto COMMAND passman_crypto_test)

    add_executable(passman_vault_test
        tests/vault_test.cpp
        src/crypto.cpp
        src/entry.cpp
        src/export.cpp
        src/vault.cpp
    )

    target_include_directories(passman_vault_test PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        "${SODIUM_INCLUDE_DIR}"
    )

    target_link_libraries(passman_vault_test PRIVATE
        "${SODIUM_LIBRARY}"
    )

    target_compile_options(passman_vault_test PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )

    add_test(NAME passman.vault COMMAND passman_vault_test)

    add_executable(passman_entry_test
        tests/entry_test.cpp
        src/entry.cpp
    )

    target_include_directories(passman_entry_test PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
    )

    target_compile_options(passman_entry_test PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )

    add_test(NAME passman.entry.serialization COMMAND passman_entry_test)

    add_executable(passman_password_test
        tests/password_test.cpp
        src/password.cpp
    )
    target_include_directories(passman_password_test PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        "${SODIUM_INCLUDE_DIR}"
    )
    target_link_libraries(passman_password_test PRIVATE
        "${SODIUM_LIBRARY}"
    )
    target_compile_options(passman_password_test PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
    add_test(NAME passman.password COMMAND passman_password_test)

    add_executable(passman_cli_test
        tests/cli_test.cpp
        src/cli.cpp
    )
    target_include_directories(passman_cli_test PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
    )
    target_compile_options(passman_cli_test PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
    add_test(NAME passman.cli.parser COMMAND passman_cli_test)

    add_executable(passman_search_test
        tests/search_test.cpp
        src/commands.cpp
        src/clipboard.cpp
        src/clipboard_timer.cpp
        src/crypto.cpp
        src/entry.cpp
        src/export.cpp
        src/password.cpp
        src/vault.cpp
    )
    target_include_directories(passman_search_test PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        "${SODIUM_INCLUDE_DIR}"
    )
    target_link_libraries(passman_search_test PRIVATE
        "${SODIUM_LIBRARY}"
    )
    target_compile_options(passman_search_test PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
    add_test(NAME passman.search COMMAND passman_search_test)
endif()


# sanitizers
if(PASSMAN_ENABLE_SANITIZERS)

    foreach(target_name
        passman
        passman_crypto_test
        passman_vault_test
        passman_entry_test
        passman_password_test
        passman_cli_test
        passman_search_test)
        if(TARGET ${target_name})
            target_compile_options(${target_name} PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
            )
            target_link_options(${target_name} PRIVATE
                -fsanitize=address,undefined
            )
        endif()
    endforeach()

endif()