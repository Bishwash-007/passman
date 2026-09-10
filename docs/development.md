# Development

Passman requires CMake 3.20+, a C++20 compiler, libsodium, and macOS terminal
tools. Configure with:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Sanitizers are enabled with `-DPASSMAN_ENABLE_SANITIZERS=ON`. The source is
organized into `include/`, `src/`, and `tests/`; CTest targets cover CLI,
crypto, vault, entry serialization, password generation, and search.

Keep changes narrow, preserve warning-clean builds, add tests for behavior
changes, and update security documentation for cryptographic or secret-lifetime
changes. Never use real vaults or credentials in tests.
