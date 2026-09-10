# Passman

Passman is a local-first password manager written in C++20 for macOS Apple
Silicon. It stores one encrypted vault on disk and provides an interactive CLI
for entry management, search, password generation, clipboard copying, import,
export, and master-password rotation.

> **Status: v1.0.0 release candidate.** The supported and validated target is
> macOS arm64. Linux and Windows are unverified.

## Features

- Argon2id key derivation through libsodium.
- Authenticated encryption with `crypto_secretbox_easy`.
- Atomic owner-only vault writes and vault format versioning.
- Add, list, get, search, delete, import, and export operations.
- Secure random password generation.
- Master-password rotation with a fresh salt and nonce.
- 30-second inactivity lock and clipboard cleanup.
- CTest coverage plus AddressSanitizer and UndefinedBehaviorSanitizer builds.

## Architecture

```text
CLI / interactive parser
          |
       Session
          |
       Commands
          |
        Vault ---- Entries
          |
        Crypto
          |
   encrypted vault.dat
```

See [docs/architecture.md](docs/architecture.md) for component ownership and
[docs/cryptography.md](docs/cryptography.md) for the cryptographic construction.

## Requirements

- macOS on Apple Silicon (arm64) for the supported release target.
- CMake 3.20 or newer.
- A C++20 compiler (Apple Clang or Clang).
- libsodium (`brew install libsodium`).
- macOS `pbcopy` and `pbpaste` for clipboard support.

## Build and install

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
cmake --install build-release --prefix "$HOME/.local"
```

The install target installs only the `passman` executable. Ensure the chosen
prefix's `bin` directory is on `PATH`.

## Usage

Run `passman` from the directory containing the vault. The default vault path
is `./vault.dat`; the current CLI intentionally does not read a global
configuration file.

```text
passman --help
passman
passman generate 32
```

Interactive commands:

```text
add <site> [--generate]   get <site> [--copy]
list                      search <query>
delete <site>             change-master
generate [length]         export <path>
import <path>             lock
help
```

Exports are deliberately unencrypted and are never overwritten. Treat them as
secrets and remove them securely after use.

For a complete first-run walkthrough, command reference, backup guidance, and
error reference, see [docs/usage.md](docs/usage.md).

## Testing and development

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

cmake -S . -B build-sanitize -DPASSMAN_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
```

The project layout and contribution process are described in
[docs/development.md](docs/development.md) and [CONTRIBUTING.md](CONTRIBUTING.md).

## Security model and limitations

The vault provides confidentiality and tamper detection when the master
password is strong and the operating system and filesystem are trusted.
Passman cannot protect against malware, keyloggers, a compromised account,
memory inspection, an attacker who can use the unlocked process, or physical
compromise of the device. Plaintext exports and clipboard contents are outside
the encrypted-vault boundary. Read [SECURITY.md](SECURITY.md) before using
Passman with important data.

Back up the encrypted `vault.dat` using a protected backup mechanism. A backup
does not replace the master password, and old backups may remain decryptable
with an older password after rotation.

## Release

The release archive is produced for macOS arm64 only. Verify an archive with:

```sh
shasum -a 256 -c SHA256SUMS
```

See [CHANGELOG.md](CHANGELOG.md) for v1.0.0 changes and
[docs/vault-format.md](docs/vault-format.md) for the on-disk format.
