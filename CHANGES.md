## 0.5.1

### Features

- #2296, #2309 - UX: Add configurable sidebar position (thanks @kyleshevlin!)
- #2318 - CLI: Add option to override default renderer selection
- #2303 - Editor: Allow changing filetypes for buffers (fixes #782)
- #2328 - Editor: Customizable line-height (fixes #966)
- #2373 - Vim: Macro indicator in status bar
- #2377, #2402 - Editor: Add yank highlights
- #2383 - Editor: Simple image renderer (fixes #474)
- #2332 - Extensions: Add XML extension
- #2415 - Search: Move search to sidebar
- #2414 - Explorer: Add explorer.autoReveal configuration (fixes #2231 - thanks @caleywoods!)
- #2456, #2476, #2492, #2502, #2498, #2519, #2524, #2556 - Vim: Modal window navigation across UI panes
- #2430, #2497 - Integrate Sparkle auto-update framework (thanks @zbaylin!)
- #2543 - Configuration: Implement per-filetype configuration
- #2555, #2559, #2572, #2577, #2578 - Editor: Initial word-wrap implementation (fixes #1444)
- #2532 - Language Support: Implement find-all-references
- #2581 - Completion: Initial keyword completion
- #2613 - Editor: Add `:scrolloff`/`editor.cursorSurroundingLines` setting
- #1566 - Menus: Integrate `fzy` for faster, more accurate fuzzy finding (thanks @CrossR!)
- #2658 - Extensions: Add dash shell (thanks @Dark-Matter7232!)
- #2664 - Editor: Implement double-click to select token
- #2646 - Experimental: Initial code-lens implementation
- #2690 - Vim: Add goto-outline command 'gO'

### Bug Fixes

- #2283 - Windows: Use ANGLE / Direct3D (fixes #1522, #1809, #1842)
- #2301 - TextMate: Fix scope matching precendence (fixes #1933)
- #1716 - Input: Handle Unicode characters properly (thanks @fanantoxa!)
- #2317 - Vim: Tab wrapping with gt/gT (fixes #2230)
- #2307 - TextMate: Fix crash when trying to load non-existent grammar
- #2300 - Completion: Manual invoke fixes
- #2325 - Extensions: Tsconfig pops up randomly (fixes #2305)
- #2351 - Vim: Crash on `:enew` (fixes #2349)
- #2364 - Vim: Show pending operator in status bar
- #2366 - SCM: Fix dialog popping up on save in non-git repos (#2366)
- #2369 - Vim: Fix `:set relativenumber`, `:set number` (fixes #888)
- #2371 - Completion: Don't show empty hover windows (thanks @CrossR!)
- #2376 - Vim: Fix `:colorscheme` ex cmd
- #2393 - Extensions: Fix URI serialization on Windows (fixes #2282)
- #2408 - Pane: Fix toggle problems pane
- #2429 - Vim: Fix visual-in motion (viw/vi") commands (fixes #2413)
- #2443 - Input: Fix control+backspace handled in text-inputs (fixes #2281 - thanks @fiddler!)
- #2457 - UX: Titlebar not draggable from icon / text (fixes #2449)
- #2458 - Windows: Set icons for application executables (fixes #2445)
- #2478 - Extensions: Fix race condition in search subscription (fixes #2478)
- #2509 - Extensions: Fix blockers for vscode-spell-checker (#2481)
- #2503 - UX: Use opacity to make the focused pane more obvious (#2503)
- #2515 - Search: Find-in-files not getting focus with hot key (fixes #2474)
- #2516 - Terminal: Switching to normal mode is not working (fixes #2504)
- #2518 - Input: Add some readline shortcuts (thanks @fratajczak!)
- #2529 - SCM: Only show active git branch indicator (fixes #2487)
- #2526 - Input: Strip `\r` when pasting text with Control+V (thanks @ShazzAmin!)
- #2547 - Editor: Reopen after close is not working (fixes #2421)
- #2548 - UX: Fix focus issues when sidebar is on the right
- #2552 - Extensions: Fix crash when searching for extensions w/o network (fixes #2484)
- #2549 - UX: Remove duplicate border on sidebar
- #2404 - Input: Support macOS shortcuts in input (thanks @fiddler!)
- #2553 - Input: Fix intermediate shift-key breaking key chords
- #2574 - Vim: Fix flakiness in cursor positioning after leaving insert mode (fixes #2566, #2268, #1194, and #831)
- #2588 - Terminal: Fix cursor jump when typing (fixes #2374)
- #2597 - Editor: Fix scroll/viewport reconciliation (fixes #760)
- #2600 - Explorer: Fix issue toggling in some configurations (fixes #2594)
- #2609 - Terminal: Use login shell for macOS (fixes #1548 - thanks @CrossR!)
- #2607 - Configuration: Create config directory if necessary
- #2608 - Terminal: Fix `LD_LIBRARY_PATH` issues on Linux (fixes #2040)
- #2610 - Terminal: Opacity / active state not set correctly (fixes #2565)
- #2634 - Extensions: On OSX, `PATH` not set for extension host / vim on startup (fixes #1161)
- #2619 - Editor: Fix horizontal scroll when using go-to-definition or search (fixes #2590)
- #2638 - Input: Fix extra key needing to be pressed for built-in commands (fixes #1691)
- #2647 - UX: Clicking on dock buttons should toggle pane (fixes #2639)
- #2648 - Extensions: Fix `ocamllsp` detection in OPAM projects (fixes #2641)
- #2630 - Vim: Fix tab completion in command line for multiple arguments (fixes #2624 - thanks @MichaelBelousov!)
- #2661 - Editor: Fix undo/redo issues with auto-closing pairs (fixes #2635)
- #2662 - Vim: Fix some issues with toggle-comment operator (fixes #2652)
- #2651 - Vim: Fix escape key handling in replace mode (fixes #2644)
- #2670 - OSX: Fix launching from dock (#2659)
- #2668 - Vim: Handle simple remaps (fixes #595)
- #2684 - Extensions: Fix TabNine extension not installing
- #2697 - Vim: Fix macro with global command (fixes #934)
- #2704 - Vim: Implement initial `getchar()` handler 
- #2704 - Vim: Fix normal commands applied across ranges (fixes #2704)
- #2719 - Bufffers: Buffer metadata not getting updated on switch to existing buffer (fixes #2605 - thanks @glennsl!)

### Performance

- #2563 - Editor: Remove shaping on all lines
- #2710 - Editor: Migrate smooth-scrolled from hook-based to subscription animation
- #2717 - Editor: Migrate yank highlights from hook-based to subscription animation
- #2718 - Status Bar: Remove expiry timer

### Documentation

- #2354 - Update modal-editing-101 (thanks @rogererens!)
- #2361 - Add render whitespace documentation (fixes #2346)
- #2382 - Add information about license key bounty
- #2500 - Fix release steps (thanks @CrossR!)
- #2623 - Improve build docs and dependencies (fixes #2611 - thanks @CrossR!)

### Infrastructure / Refactoring

- #2336 - Upgrade JetBrains Mono from 1.03 to 2.001 (thanks @zbaylin!)
- #2348 - Byte/Character index differentation in type system
- #2416 - Update project language to 2.5 (thanks @zbaylin!)
- #2422 - Extensions: Upgrade vscode-exthost -> 1.46.0
- #2423 - Extensions: Upgrade vscode-exthost -> 1.47.1
- #2674 - Move package definitions to dune-project (thanks @rgrinberg!)
- #2683 - Extensions: Upgrade vscode-exthost -> 1.50.1

## 0.5.0

- Changelog started
