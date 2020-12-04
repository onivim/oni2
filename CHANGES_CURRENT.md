## vNext

### Features

- #2712 - Completion: Add support for additionalTextEdits (fixes #2360)
- #2687, #2756 - Implement a Reason alternative to atom/keyboard-layout (thanks @zbaylin!)
- #2763 - Explorer: Open Folder button
- #2765 - Vim: Hook up `:messages` and `:messages clear` ex commands
- #2783 - Promote editor.wordWrap from experimental (fixes #1444)

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

### Performance

- #2736 - Switch to model-based animation for notification yOffset
- #2407 - Migrate color transitions to model-based animation
- #2766 - Remove initial dock aniamtion (fixes #1131)

### Documentation

- #2731 - Add explorer context to key-bindings.md (thanks @tristil!)
- #2797 - Add add-to-path instructions for OSX to getting started

### Infrastructure / Refactoring

- #2759 - Fix `esy @release run` command on CI (thanks @zbaylin)


