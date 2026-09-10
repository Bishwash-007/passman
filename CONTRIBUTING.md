# Contributing

Passman is a security-sensitive C++ project. Open a focused branch and pull
request for each change. Explain behavior changes and include regression tests
where practical.

Before submitting:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Use C++20, existing warning flags, RAII, explicit bounds checks, and the
existing libsodium APIs. Security-sensitive changes should update the relevant
documentation and explain key, nonce, memory, and error-path behavior.

Never commit passwords, vault files, plaintext exports, private keys, tokens,
personal paths, or other private data. Do not weaken authentication or disable
sanitizers to make a test pass. Review [SECURITY.md](SECURITY.md) before
reporting a vulnerability; do not disclose an exploitable issue in a public
issue.
