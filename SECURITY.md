# Passman security and threat model

## Scope

Passman protects the confidentiality and integrity of the encrypted local
vault against an offline reader who does not know the master password, assuming
libsodium, the operating system, and the filesystem have not been compromised.
It is not a complete endpoint-security product.

It does not protect against malware, keyloggers, a hostile or already-unlocked
desktop session, memory scraping, malicious clipboard readers, compromised
dependencies, weak/reused master passwords, or physical attacks that bypass the
device's protections. A user with write access can delete or replace the vault,
even though authenticated encryption detects tampering when it is opened.

## Design

- A random libsodium salt is generated for each vault and master-password
  rotation.
- The master password is processed with Argon2id (`crypto_pwhash`,
  moderate operations and memory limits) to derive a 32-byte key.
- Entries are serialized, encrypted with libsodium
  `crypto_secretbox_easy`, and authenticated with its MAC.
- Each save uses a fresh random secretbox nonce. The nonce is stored with the
  ciphertext and is never treated as secret.
- Randomness comes from libsodium `randombytes_buf` and
  `randombytes_uniform`.
- The versioned header contains `PMV1`, version, salt, nonce, and ciphertext.
  Legacy header-only vaults remain readable.

Keys and serialized plaintext are explicitly wiped on the principal success and
failure paths with `sodium_memzero`. C++ strings and allocator behavior cannot
provide an absolute guarantee against every copy made by the runtime.

## Runtime behavior

The interactive session locks after 30 seconds without input and on manual
lock, EOF, or session destruction. Clipboard passwords are cleared after 30
seconds when the clipboard still contains the copied value; cleanup is best
effort because the operating system clipboard is outside the vault.

Vault writes use a temporary owner-only file and replacement rename. Files are
restricted to the owner on supported POSIX filesystems. The filesystem,
directory permissions, backups, snapshots, and swap are environmental
assumptions.

Password rotation derives a new key and salt, encrypts the current entries with
a new nonce, atomically replaces the vault, and only then replaces the
in-memory key. Existing backups are not re-encrypted.

## Import and export

The export format is plaintext by design. Export only to a protected location,
do not share it, and remove it after use. Imports validate the binary format and
reject malformed or duplicate entries before changing the vault. Import files
can still contain sensitive passwords and should be treated as untrusted input.

## Operational guidance

Use a unique, high-entropy master password; protect the device account; keep
encrypted backups; avoid unattended unlocked sessions; and do not run Passman
from a directory writable by untrusted users. Code signing and notarization are
not performed by this repository's local release script and must be handled by
the distributor.
