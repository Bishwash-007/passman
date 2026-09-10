# Vault format

All integer lengths are unsigned little-endian 32-bit values.

Current files are:

```text
4 bytes   magic: "PMV1"
1 byte    format version: 1
16 bytes  Argon2id salt (libsodium crypto_pwhash_SALTBYTES)
24 bytes  secretbox nonce (libsodium crypto_secretbox_NONCEBYTES)
N bytes   secretbox ciphertext, including the authentication MAC
```

The ciphertext decrypts to an entry collection:

```text
4 bytes       entry count
repeat count:
  4 bytes + UTF-8/raw bytes  site
  4 bytes + UTF-8/raw bytes  username
  4 bytes + raw bytes        password
  4 bytes + UTF-8/raw bytes  notes
```

Field lengths and the entry count are bounds-checked; trailing bytes are
rejected. A legacy file without the `PMV1` magic/version is read as the old
format: salt, nonce, then ciphertext. New saves always write the versioned
format. Unsupported current versions and authentication failures are rejected.

The export format is separate and begins with `PME1`, followed by a 32-bit
entry count and the same four length-prefixed fields. It is plaintext and is
not a vault format.
