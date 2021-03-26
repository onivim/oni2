### Features 

- #3284 - Definition: Add 'editor.action.revealDefinitionAside' command (fixes #3261)

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

### Performance

- #3316 - Transport: Re-use Luv.Buffer.t when possible for reads

### Documentation

- #3293 - Formatting: Initial formatting documentation
- #3296 - Emmet: Add documentation on using Emmet for tsx/jsx files (fixes #3283)

### Refactoring

- #3303 - Language Support: Move language metadata into Feature_LanguageSupport (related #3288)
- #3330 - Diagnostics: Add regression test for #3233

### Infrastructure

- #3319 - Dependency: esy-macdylibbundler -> 0.4.5001 to support Big Sur (thanks @brdoney !)
- ##3313 - Packaging: Fix macOS Big Sur release bundling issues (fixes #2813, thanks @brdoney !)
