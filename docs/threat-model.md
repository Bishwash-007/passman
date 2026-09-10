# Threat model

Passman assumes the local user account, process, filesystem, and libsodium
installation are trusted while the vault is locked. Its primary offline
adversary can read a copied vault but does not know the master password.
Argon2id slows guessing and secretbox authentication detects modification.

Out of scope are keyloggers, malware, clipboard monitors, memory forensics,
root/admin compromise, an attacker using an unlocked session, weak passwords,
and physical attacks that defeat device protections. Backups, swap, shell
history, plaintext exports, and terminal/desktop capture require separate
operational controls.
