### Features 

- #3613 - Input: add Emoji & Symbols panel on macOS
- #3641 - Search: add button to enable RegEx

### Bug Fixes

- #3597 - Registration: Trim license key before submitting
- #3431 - Extensions: Serialize file system errors correctly to extensions (fixes #3418)
- #3600 - OSX: Fix crash on selecting native menu item
- #3604 - Editor: Fix CodeLens blocking mousewheel
- #3606 - Vim: Fix hang when using `function` in `experimental.viml`
- #3616 - OSX: Fix crash on IME switch (fixes #3614)
- #3621 - Sneak: Grab editor focus when using sneak to jump to an editor (fixes #2569)
- #3630 - Editor: Cursor / line number disappearing when deleting entire buffer (fixes #3629)
- #3637 - Extensions: Unable to enter API key for WakaTime (fixes #3619)
- #3638 - Editor: Add clear search highlights default keybinding (fixes #3636)
- #3645 - Terminal: Auto-scroll when key is pressed

### Performance

### Documentation

- #3601 - Visual Mode: Add note about switching to block mode on Windows / Linux (thanks @rogererens !)
- #3620 - Building: Add steps for Docker based build.
- #3626 - Keybindings: Document nextEditor/previousEditor bindings (thanks @paul-louyot !)

### Refactoring

- #3595 - Dependency: OCaml -> 4.12
- #3624 - Dependency: SDL2 -> 2.0.14 (thanks @zbaylin !)
- #3632 - Dependency: revery -> 79c2572
- #3639 - Dependency: Sparkle -> 1.26.0

### Infrastructure
