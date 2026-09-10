# Architecture

```text
CLI parser -> Session -> Commands -> Vault -> Crypto -> vault.dat
                                      |
                                    Entry
```

- `cli` parses process arguments and interactive command lines and prints
  usage.
- `main` initializes libsodium, chooses the local vault, and starts the session.
- `session` owns the unlocked vault, tracks activity, and performs auto-lock.
- `commands` implements user-facing operations without exposing passwords in
  normal output.
- `vault` owns entries, key material, persistence, permissions, rotation, and
  atomic saves.
- `entry` encodes and decodes the plaintext entry collection.
- `crypto` wraps libsodium key derivation and secretbox encryption.
- `password` reads hidden terminal input and generates random passwords.
- `clipboard` and `clipboard_timer` integrate with macOS `pbcopy`/`pbpaste`.
- `export` handles the separate, explicitly plaintext PME1 format.

The executable is linked directly to libsodium. No service, database, or
network backend is required.
