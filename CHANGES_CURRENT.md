### Features 

- #3613 - Input: add Emoji & Symbols panel on macOS
- #3641 - Search: add button to enable RegEx
- #3654 - Search: add button to enable case-sensitivity
- #3661 - Search: add include/exclude file boxes
- #3705 - Editor: add right click menus
- #3718 - Completion: Add `editor.suggest.itemsToShow` setting (fixes #3712)
- #3736 - Search: add default keys to go to next / previous search result (fixes #3713)

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
- #3652 - Syntax: Fix highlighting being lost after movement (fixes #3647)
- #3607 - Vim: Fix crash when specifying empty guifont (fixes #3605)
- #3665 - CodeLens: Fix overlapping codelens with editor text (fixes #3663)
- #3668 - UX: Toggle drop-down menu on click (fixes #3569 - thanks @sijad !)
- #3571 - Extension: Implement $tryApplyEdits (fixes #3545)
- #3335 - Extension: Fix selection bounds (fixes #3335)
- #3699 - Vim: Respect silent flag for output-producing commands (fixes #3680)
- #3692 - CodeLens: Fix delegated commands not executing (related #2380)
- #3702 - OSX: Fix crash on open-with (fixes #3698)
- #3703 - Sidebar: Fix window navigation to sidebar when closed or zen mode (related #3681)
- #3709 - Extension: Fix activation error with nim extension (fixes #3685)
- #3612 - Input: Fix unicode parsing for keybindings (fixes #3599)
- #3717 - Terminal: Fix mousewheel / trackpad scroll direction (fixes #3711)
- #3719 - Input: Add 'editorFocus' context key (fixes #3716)
- #3732 - Input: Fix remapped keys executing in wrong order (fixes #3729)
- #3747 - Layout: Implement Control+W, C binding (related #1721)
- #3746 - Extension: Fix edit application in trailing spaces plugin
- #3755 - Vim: Fix extra 'editor tab' with `:tabnew`/`:tabedit` (fixes #3150)
- #3753 - Extension: Don't bubble up extension runtime errors to notifications

### Performance

- #3603 - UX: Fix a couple of orphaned animation timers (related #2407)

### Documentation

- #3601 - Visual Mode: Add note about switching to block mode on Windows / Linux (thanks @rogererens !)
- #3620 - Building: Add steps for Docker based build.
- #3626 - Keybindings: Document nextEditor/previousEditor bindings (thanks @paul-louyot !)
- #3740 - Getting Started: Fix dead links (thanks @Doerge !)
- #3754 - Settings: Add `workbench.activityBar.visible` to documentation (fixes #3720)

### Refactoring

- #3595 - Dependency: OCaml -> 4.12
- #3624 - Dependency: SDL2 -> 2.0.14 (thanks @zbaylin !)
- #3632 - Dependency: revery -> 79c2572
- #3639 - Dependency: Sparkle -> 1.26.0
- #3653 - Dependency: revery -> 9ec44ff (fixes #3646)
- #3688 - Dependency: esy-skia -> 1c81aac
- #3744 - Dependency: vscode-exthost -> 1.56.2 (fixes #3737)
- #3745 - Dependency: vscode-exthost -> 1.57.1

### Infrastructure

- #3721 - Packaging - Linux: Bundle compiled glib settings (fixes #3706)
