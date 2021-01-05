### Features

- #2866 - Extensions: Show ratings / download count in details view (fixes #2866)
- #2889 - Extensions: Include HTML, JSON, PHP, and Markdown language servers
- #2849 - UX: Initial menu bar on Windows / Linux (fixes #1255)
- #2596 - Editor: File preview when clicking on files (thanks @fanantoxa!)

### Bug Fixes

- #2845 - Workspace: Opening a file should not always open a folder (fixes #1983)
- #2842 - CLI: AppImage - fix argument parsing in `AppRun` (fixes #2803)
- #2839 - Extensions: CodeLens - fix lens persisting when line is deleted
- #2844 - Vim: Fix `:tabnew`/`:new`/`:vnew` behavior (fixes #1455, #2753, #2843)
- #2846 - UX: Editor Tabs - horizontal scrolling on trackpad is reversed (thanks @SeitaHigashi!)
- #2865 - OSX: Fix drag-and-drop on dock icon (fixes #2855)
- #2867 - Editor: Fix viewport shifting when deleting lines with codelens
- #2868 - SCM: Diff markers not showing up in gutter (fixes #2857)
- #2869 - Hover: Fix hover pop up while scrolling via mousewheel
- #2871 - Vim: Fix `ctrl+o` behavior in insert mode (fixes #2425)
- #2854 - Extensions: Signature Help - fix overlay staying open in normal mode
- #2877 - Vim: Remove conflicting `ctrl+b` binding on Windows / Linux (fixes #2870)
- #2878 - Input: Treat `Ctrl+[` as `Escape` everywhere
- #2879 - Editor: Correct diff marker rendering in presence of codelens
- #2891 - Vim: Fix count behavior for L/H jumps (fixes #2882)
- #2895 - Completion: Fix crash on long (>1024 character) completion matches (fixes #2892)
- #2905 - CLI: HealthCheck - Re-enable output logging
- #2907 - Editor: Add configuration for document highlights and use proper theme color
- #2902 - Input: Fix remaps for characters w/o scancode (fixes #2883)
- #2908 - Input: Fix no-recursive remap behavior (fixes #2114)
- #2917 - Extensions: CodeLens - fix extraneous animation with multiple providers
- #2628 - Input: Right arrow key treated as PageUp
- #2929 - Input: Fix intermittent crash when scrolling with the mouse (fixes #2919)
- #2927 - Input: Windows - fix crash in entering Unicode character (fixes #2926)

### Performance

- #2852,#2864 - Performance: Batch editor / codelens animations
- #2901 - Bundle Size: Remove unused Selawik and Inconsolata fonts
- #2932 - Language Features: Debounce high-frequency subscriptions

### Documentation

- #2874 - Languages: Fix extension ids for `exuberant-ctags` and `clangd` (fixes #2872)
- #2893 - Building: Add Xorg dependencies for Linux (thanks @marcinkoziej!)

### Infrastructure / Refactoring

- #2853 - Input: Add APIs for querying contextually available bindings and consumed keys
- #2886 - Extensions: Upgrade vscode-exthost -> 1.51.0
- #2887 - Build: Remove hardcoded extension host version; pull from package
- #2888 - Extensions: Upgrade vscode-exthost -> 1.52.1
- #2894 - Build: Linux - Fix permission problem removing setup.json (thanks @marcinkoziej!)
- #2889 - Extensions: Upgrade extensions to 1.52.1
- #2904 - Build: Fix warnings in macOS build (thanks @zbaylin!)
