# Usage guide

## First run

Run `passman` from any directory:

```sh
passman
```

Passman prompts for a master password. If
`~/.local/share/passman/vault.dat` does not exist, it creates a new encrypted
vault with owner-only file permissions. If it exists, Passman attempts to
unlock it with the supplied password.

Use a unique, high-entropy master password. Passman does not provide password
recovery; losing it means losing access to the vault.

## Interactive commands

After unlocking, type `help` to display the command list.

### Add an entry

```text
add github.com
```

Passman prompts for the username, password, and notes. To generate a random
password instead:

```text
add github.com --generate
```

Sites must be unique within a vault. Adding a duplicate site is rejected.

### List sites

```text
list
```

Lists saved site names without displaying usernames, passwords, or notes.

### View an entry

```text
get github.com
```

Displays the site, username, masked password, and notes. To copy the password
to the macOS clipboard for up to 30 seconds:

```text
get github.com --copy
```

The clipboard timer only clears the clipboard if it still contains the value
Passman copied. `lock` also requests clipboard cleanup.

### Search

```text
search github
```

Search is case-insensitive and checks site, username, and notes. Exact site
matches are ranked first, followed by site prefixes and then other matches.
Passwords are never searched or displayed in search results.

### Delete

```text
delete github.com
```

Deletion is immediate in memory and is persisted by Passman. Deleting a
nonexistent site returns an error.

### Generate a password

```text
generate
generate 32
```

Generated passwords contain at least one uppercase letter, lowercase letter,
digit, and symbol. Valid lengths are 8 through 256 characters.

### Change the master password

```text
change-master
```

Passman prompts for and confirms the new password. It generates a new salt and
encryption key, encrypts the current entries with a fresh nonce, and atomically
replaces the vault. The old password cannot unlock the rotated vault.

### Import and export

```text
export /protected/path/passman-export.pme
import /protected/path/passman-export.pme
```

Exports are unencrypted and contain every entry, including passwords. Passman
does not overwrite an existing export path. Protect exports like passwords and
remove them after use. Imports reject malformed files and duplicate sites.

### Lock and exit

```text
lock
```

Locks the current session. Passman also locks after 30 seconds without input,
when standard input reaches EOF, or when the session ends. Run `passman` again
to unlock.

## Vault location

The vault is stored at:

```text
~/.local/share/passman/vault.dat
```

Passman creates the `~/.local/share/passman` directory when needed. It does not
currently support a global configuration file or a command-line vault-path
option.

## Backup and recovery

Back up the encrypted `vault.dat` to a protected location. Test that backups
are readable before relying on them. Backups made before a master-password
rotation remain encrypted with the previous password, so rotate or delete old
backups according to your threat model.

Passman cannot recover a forgotten master password.

## Common errors

- **Master password cannot be empty**: provide a non-empty password.
- **Incorrect password or corrupted vault**: verify the password and restore a
  known-good backup if the file may be damaged.
- **Entry already exists**: use a unique site name.
- **No entry found**: check the exact site name with `list` or use `search`.
- **Export destination already exists**: choose a new path; exports are never
  overwritten automatically.
- **Could not open clipboard**: verify that macOS `pbcopy` and `pbpaste` are
  available and that the session permits clipboard access.

For security reporting, follow [SECURITY.md](../SECURITY.md). For building and
testing, see [development.md](development.md).
