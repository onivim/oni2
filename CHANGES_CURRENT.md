### Features

- #2712 - Completion: Add support for additionalTextEdits (fixes #2360)
- #2687, #2756 - Implement a Reason alternative to atom/keyboard-layout (thanks @zbaylin!)
- #2763 - Explorer: Open Folder button
- #2765 - Vim: Hook up `:messages` and `:messages clear` ex commands
- #2783 - Promote editor.wordWrap from experimental (fixes #1444)
- #2774 - Extensions: Implement update check (also fixes #2748)
- #2801 - Extensions: CodeLens - Handle multiple codelens on same line
- #2812 - CLI: Add `+` and `-c` arguments to run Vim ex commands (fixes #1861)
- #2817 - Snippets: Initial snippet parser (related #160)
- #2818 - Vim: Initial `:!` command output implementation (fixes #1889, #1909)
- #2832 - CodeLens: Implement resolve provider

### Bug Fixes

- #2733, #2737 - Several fixes around linewise scroll (control+e, control+y)
- #2738 - Input: `<Nop>` key is not being bound correctly
- #2729 - Input: Fix synchronization of cursor position when running ex command (fixes #2726)
- #2744 - Fix titlebar height on macOS Big Sur (thanks @zbaylin!)
- #2728 - Extensions: Fix error when searching (fixes #2723)
- #2711 - Sidebar: Don't show sidebar if no folder is open (fixes #2084)
- #2760 - Completion: Fix crash on details error (fixes #2752)
- #2761 - OSX: Fix crash with Cmd+P on first launch (fixes #2742)
- #2772 - Editor: Fix several bugs with the active-indent renderer
- #2786 - Extensions: Fix download from open-vsx
- #2785 - Windows: Fix intermittent crash in `Luv.Process.spawn`
- #2792 - Vim: Handle count for insert mode commands (`i`/`a`, etc) (fixes #809, #2190)
- #2802 - Editor: Fix crash when moving mouse in empty buffer (fixes #2800)
- #2758 - Vim: Insert literal not working in command-line mode (fixes #2747)
- #2806 - Windows: Add open-directory command to windows installer (fixes #2046)
- #2807 - Terminal: Implement paste in insert mode (fixes #2805)
- #2808 - Auto-Update: Fix changelog display (fixes #2787)
- #2809 - Extensions: Warn on open-vsx public namespaces (fixes #2345)
- #2810 - Extensions: Fix intermittent request failure getting extension details
- #2811 - Extensions: Show logo image for remote extensions
- #2819 - Auto-Update: Set default update channel based on build type
- #2822 - Build: Use development `Info.plist` that sets `NSSupportsAutomaticGraphicsSwitching` (fixes #2816)
- #2826 - Search: Default to simple string search for find-in-files (fixes #2821)

### Performance

- #2736 - Switch to model-based animation for notification yOffset
- #2407 - Migrate color transitions to model-based animation
- #2766 - Remove initial dock animation (fixes #1131)

### Documentation

- #2731 - Add explorer context to key-bindings.md (thanks @tristil!)
- #2797 - Add add-to-path instructions for OSX to getting started

### Infrastructure / Refactoring

- #2759 - Fix `esy @release run` command on CI (thanks @zbaylin)
- #2831 - Upgrade `ocaml-lsp` to 966a28f


