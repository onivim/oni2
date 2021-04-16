### Features 

- #3284 - Definition: Add 'editor.action.revealDefinitionAside' command (fixes #3261)
- #2881 - Extensions: Initial rename support
- #3348 - Code Actions: Implement extension host protocol for quick fix / code actions
- #3352 - Keybindings: Add default keybinding for open configuration (related #1423, thanks @LiHRaM !)
- #3381 - Language Support: Symbols - implement go-to buffer symbol menu
- #3387 - Code Actions: Implement context menu for quick fixes

### Bug Fixes

- #3289 - Extensions: Fix logs location path provided to extension (related #3268)
- #3290 - Symbols: Fix issues focusing nodes in symbol outline view (fixes #2002)
- #3292 - Vim: Fix control+d/control+u behavior in explorer
- #3295 - Formatting: Make formatting notifications ephemeral
- #3297 - Diagnostics: Fix potential crash when deleting lines with diagnostics
- #3298 - Completion: Sort ordering improvements (related #3283)
- #3301 - Formatting: Fix crash in default formatter with negative indentation levels
- #3302 - Auto-Indent: Implement $setLanguageConfiguration handler (related to #3288)
- #3300 - Formatting: Fix format edits containing a trailing newline (related to #3288)
- #3307 - UX: Bring back 'workbench.tree.indent' configuration setting (fixes #3305)
- #3309 - Completion: Fix off-by-one keyword / snippet completion
- #3310 - Snippets: Fix parse error for printf/sprintf snippets
- #3287 - Search: Initiate search automatically when text is selected (fixes #3277)
- #3308 - Completion: Allow case-insensitive matches in scoring (fixes #3136)
- #3317 - Extensions: Fix haskell files at root not loading language integration (related #2380)
- #3318 - Completion: Show full incomplete results list (fixes #2583)
- #3329 - Formatting: Fix trailing newline being introduced by some providers (fixes #3320)
- #3331 - Editor: Fix crash when manipulating Unicode characters
- #3327, #3329 - Formatting: Fix trailing newline being introduced by some providers (fixes #3320)
- #3338 - SCM: Show changes badge in dock (fixes #3315)
- #3341 - Diagnostics: Use one-based positions in UI
- #3342 - Completion: Use completion kind instead of insert text rules for sorting
- #3346 - Extensions: Fix parse errors for progress and updateConfigurationOption APIs (related #3321)
- #3326 - Lifecycle: Delay process termination until cleanup actions have run (fixes #3270, thanks @timbertson !)
- #3359 - SCM: Don't show diffs for untracked or ignored files (fixes #3355)
- #3361 - Clipboard: Add command+v binding for paste in normal mode (fixes #3353)
- #3364 - Input: Add context key to differentiate sidebar panels
- #3365 - Explorer: Add manual refresh command and experimental `files.useExperimentalFileWatcher` setting (fixes #3350)
- #3371 - Explorer: Fix regression in auto-focus behavior
- #3376 - Vim: Missing Control+W,W bindings (related #1721)
- #3379 - Rename: Handle parsing 'rejectReason'
- #3382 - Input: Handle `<capslock>` key
- #3389 - Editor: Render diagnostics with squiggly lines (fixes #2827)
- #3394 - Extensions: Fix error parsing extension manifest with boolean when express (related #3388)
- #3390 - Diagnostics: Some diagnostics wouldn't show in hover UI (related #3231)
- #3403 - Code Actions: Add 'editor.lightBulb.enabled' configuration setting
- #3396 - Code Actions: Fix some issues around light bulb rendering
- #3404 - UX - Explorer: Fix text overflow in explorer (fixes #3362)
- #3414 - Code Actions: Fix regression in control-p binding
- #3416 - Editor: Fix word-wrap calculation with tab characters (fixes #3372)
- #2329 - UX - Completion: Fix overflowing detail text (fixes #2264)
- #3421 - Lifecycle: Show unsaved dialog when closing last editor via mouse (fixes #417)
- #3420 - Vim: Implement C-W, C-Q binding (related #1721)
- #3422 - Input: Fix terminal key binding not working as expected (fixes #2778)

### Performance

- #3316 - Transport: Re-use Luv.Buffer.t when possible for reads
- #3374 - FileWatcher: Don't stat on file changes (related #3373)
- #3378 - libuv: Run event loop once each Revery iteration

### Documentation

- #3293 - Formatting: Initial formatting documentation
- #3296 - Emmet: Add documentation on using Emmet for tsx/jsx files (fixes #3283)
- #3411 - Vim Tips: Fix missing links (fixes #3407)

### Refactoring

- #3303 - Language Support: Move language metadata into Feature_LanguageSupport (related #3288)
- #3330 - Diagnostics: Add regression test for #3233
- #3334 - Dependency: Revery -> b746d68 (thanks @timbertson !)
- #3385 - Dependency: Revery -> 4337e2c
- #3408 - Dependency: Revery -> 373b087

### Infrastructure

- #3319 - Dependency: esy-macdylibbundler -> 0.4.5001 to support Big Sur (thanks @brdoney !)
- #3313 - Packaging: Fix macOS Big Sur release bundling issues (fixes #2813, thanks @brdoney !)
- #3369 - CI: Install python3 on CentOS dockerfile
