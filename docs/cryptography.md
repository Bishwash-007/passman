# Cryptography

Passman uses the following construction:

```text
master password + random salt
        |
        v
crypto_pwhash(Argon2id13, moderate limits)
        |
        v
32-byte secretbox key
        |
serialized entries + random nonce
        |
crypto_secretbox_easy
        |
authenticated ciphertext
```

The implementation uses `crypto_pwhash_SALTBYTES`,
`crypto_pwhash_OPSLIMIT_MODERATE`, `crypto_pwhash_MEMLIMIT_MODERATE`, and
`crypto_pwhash_ALG_ARGON2ID13`. Encryption uses
`crypto_secretbox_KEYBYTES`, `crypto_secretbox_NONCEBYTES`,
`crypto_secretbox_MACBYTES`, `crypto_secretbox_easy`, and
`crypto_secretbox_open_easy`.

Salt and nonce are stored as metadata; they do not need secrecy. A new salt,
key, and nonce are generated during password rotation. The in-memory key and
serialized plaintext are wiped on normal cleanup and principal error paths
with `sodium_memzero`. Password strings may have implementation-created copies,
so this is not a guarantee against a compromised runtime or operating system.
