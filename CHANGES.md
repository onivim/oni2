# 0.5.7 (July 2021)

### Features 

- #3613 - Input: add Emoji & Symbols panel on macOS
- #3641 - Search: add button to enable RegEx
- #3654 - Search: add button to enable case-sensitivity
- #3661 - Search: add include/exclude file boxes
- #3705 - Editor: add right click menus
- #3718 - Completion: Add `editor.suggest.itemsToShow` setting (fixes #3712)
- #3736 - Search: add default keys to go to next / previous search result (fixes #3713)
- #3733 - Quick Open: Add bindings to open in splits, not current buffer.
- #3765 - UX: Add `"window.titleBarStyle"` configuration setting

> __BREAKING:__ On Windows, the default setting is to use the `"native"` title bar.
> Set `"window.titleBarStyle": "custom"` to keep the previous behavior.

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
- #3756 - Search: Follow symlinks with ripgrep by default (fixes #1588)
- #3757 - Buffers: Fix welcome buffer appearing when splitting (fixes #3631)
- #3763 - Quickmenu: Select first item by default
- #3767 - Minimap: Fix diff marker positions on scroll (fixes #3648)

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
- #3770 - Build: Fix `esy watch` command (thanks @eEQK !)
- #3780, #3786 - Chore: Update windows codesigning certificate

# 0.5.6 (June 2021)

### Features 

- #3284 - Definition: Add 'editor.action.revealDefinitionAside' command (fixes #3261)
- #2881 - Extensions: Initial rename support
- #3348 - Code Actions: Implement extension host protocol for quick fix / code actions
- #3352 - Keybindings: Add default keybinding for open configuration (related #1423, thanks @LiHRaM !)
- #3381 - Language Support: Symbols - implement go-to buffer symbol menu
- #3387 - Code Actions: Implement context menu for quick fixes
- #3440 - Indentation: Easier space/tab indentation switching
- #3337 - Proxy: Add configuration for proxies (fixes #3314, #2660)
- #3419 - Buffers: Implement auto-save functionality (fixes #2431)
- #3430 - Buffers: Implement workbench.action.quickOpenBuffer (fixes #3413)
- #3481 - UX: Add 'workbench.tree.renderIndentGuides' setting
- #3499 - Editor: Render deprecated ranges with strikethrough (fixes #3485)
- #3506 - Theme: Add 'workbench.colorCustomizations' setting (related #3495)
- #3526 - Explorer: Enable auto-refresh by default (fixes #3399)
- #3529 - Keybindings: Allow remaps to be defined in keybindings.json
- #3539 - Buffers: Add Save without formatting commaaand (fixes #3521)

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
- #3435 - Completion: Fix various mouse interactions (fixes #3428)
- #3439 - Hover: Remove textOverflow(Ellipsis) from hover
- #3442 - Buffers: Fix regression causing control+tab menu not to stay open (related #3442)
- #3443 - App: Fix broken window positioning w/ multiple monitors (fixes #3349)
- #3451 - Terminal: Fix opening file using `oni2` from integrated terminal (fixes #2297)
- #3456 - Diagnostics: Swift diagnostics not showing in editor surface (related #3434)
- #3457 - Snippets: Fix parse error when colon is in placeholder (related #3434)
- #3464 - Vim: Fix alternate-file keybinding (fixes #3455)
- #3465 - Quickmenu: Remove darkening of background when menu is open (fixes #3459)
- #3466 - Input: Fix `gt` binding parsing (fixes #3256)
- #3474 - Explorer: Reload explorer when `files.exclude` is updated (fixes #1104)
- #3476 - UX: Ellipsize author in extensions item view
- #3475 - Buffers: Fix ctrl-tab ordering (fixes #3433)
- #3469 - CLI: Default to opening files in existing editor (fixes #1250, #2617, #2947, #3058)
- #3487 - Quickopen: Filter out spaces in query string (fixes #3473, related #3278)
- #3488 - Search: Restart search even when query hasn't changed (fixes #3477)
- #3482 - CodeLens: Make codelenses clickable (related #3468)
- #3525 - CLI: Fix crash when opening non-existent file in running instance
- #3530 - UX: Fix click inside context menu from closing context menu (fixes #3510)
- #3532 - OSX: Fix crash when opening folder with Unicode characters (fixes #3519)
- #3533 - Completion: Fix hang when detail text is large
- #3537 - Terminal: Fix pasted text showing in reverse order (fixes #3513)
- #3541 - Pane: Should grab focus when clicked (fixes #3538)
- #3547 - Completion: Add `editor.acceptSuggestionOnTab` setting
- #3556 - Pane: Fix issue with close button
- #3558 - Extensions: Clicking on search input should focus it
- #3559 - Explorer: Don't open explorer on `:cd` if `workbench.sideBar.visible` is false
- #3560 - Shell: Fix incorrect PATH on OSX when launched via Finder (fixes #3199)
- #3567 - Explorer: Fix files in NFS mount not loading (fixes #3534)
- #3568 - Explorer: Fix symlinks showing up as empty files (fixes #2657)
- #3581 - Explorer: Fix population in stat failure
- #3585 - Extensions: Fix scroll resetting while search is in progress (fixes #3551)
- #3584 - Minimap: Fix mouse scroll for small files (fixes #3563)

### Performance

- #3316 - Transport: Re-use Luv.Buffer.t when possible for reads
- #3374 - FileWatcher: Don't stat on file changes (related #3373)
- #3378 - libuv: Run event loop once each Revery iteration

### Documentation

- #3293 - Formatting: Initial formatting documentation
- #3296 - Emmet: Add documentation on using Emmet for tsx/jsx files (fixes #3283)
- #3411 - Vim Tips: Fix missing links (fixes #3407)
- #2272 - Documentation: Fix AppImage installation instructions (thanks @mkenigs !)
- #3458 - Documentation: Fix search motion key (thanks @Minnozz !)
- #3472 - Documentation: Add `node-gyp` instructions (fixes #3467)
- #3491 - Documentation: Add `--new-window` to CLI invocation (thanks @jesstelford !)

### Refactoring

- #3303 - Language Support: Move language metadata into Feature_LanguageSupport (related #3288)
- #3330 - Diagnostics: Add regression test for #3233
- #3334 - Dependency: Revery -> b746d68 (thanks @timbertson !)
- #3385 - Dependency: Revery -> 4337e2c
- #3408 - Dependency: Revery -> 373b087
- #3449 - Dependency: esy -> 0.6.10
- #3446 - Dependency: Upgrade dune, isolinear, and revery-terminal to prep for compiler upgrade
- #3447 - Terminal: Split out PTY - remove dependency on `$spawnExtHostProcess` API
- #3461 - Dependency: vscode-exthost -> 1.54.3
- #3462 - Dependency: vscode-exthost -> 1.55.2
- #3500 - Dependency: Revery -> e0c12e7 (fixes #3494)
- #3514 - Configuration: Remove legacy configuration values
- #3518 - Dependency: Revery -> bb73d21 (fixes #3504)
- #3531 - Dependency: OCaml -> 4.11
- #3540 - Dependency: Revery -> d599514 (Pick up mousewheel scaling fix)
- #3555 - Terminal: Bring in vterm/revery-terminal to monorepo
- #3594 - Dependency: Revery -> b27b0b9 (Pick up resdl_SDL_PollEvent fix)

### Infrastructure

- #3319 - Dependency: esy-macdylibbundler -> 0.4.5001 to support Big Sur (thanks @brdoney !)
- #3313 - Packaging: Fix macOS Big Sur release bundling issues (fixes #2813, thanks @brdoney !)
- #3369 - CI: Install python3 on CentOS dockerfile
- #3429 - CI: Remove libtools dependency
- #3450 - CI: Fix esy cache path after upgrade to 0.6.10
- #3452 - CI: Fix concurrent artifact upload for macOS VMs

# 0.5.5 (April 2021)

### Breaking

- #3193 - Reason: Remove `reason-language-server` executable

> NOTE: For native Reason & OCaml, see: https://onivim.github.io/docs/languages/reason-ocaml

> NOTE: For ReScript & BuckleScript, see: https://onivim.github.io/docs/languages/rescript

### Features 

- #3163 - UX: Add clear notification button (thanks @andr3h3nriqu3s11 !)
- #3246 - Formatting: Add 'editor.formatOnSave' configuration (fixes #2464)

### Bug Fixes

- #3141 - Windows: Reload keybindings on save
- #3145 - Vim: Don't crash on confirm flag with substitute ex command (fixes #1159, related #2965)
- #3142 - Windows: Explorer - Directory nodes not expanding (fixes #3092, #2213)
- #3147 - Editor: Fix minimap scroll synchronization for large files
- #3077 - Terminal: Use editor font as default (related #3062)
- #3146 - Vim: Fix command-line staying open when clicking the editor or file explorer (fixes #3031)
- #3161 - Configuration: Turn soft word-wrap on by default (fixes #3161)
- #3162 - Windows: Support opening UNC paths (fixes #3151)
- #3166 - Windows: Fix dead key input (fixes #3157)
- #3167 - Diagnostics: Show full path to trace file
- #3170 - CLI - Windows: Allocate console with `-f --silent`
- #3180 - Explorer: Fix explorer disappearing when changing into current path
- #3183 - Auto-update: Default auto-update channel should match source build
- #3184 - Explorer: Fix shrinking when changing paths
- #3160 - Windows: Reload configuration on save
- #3194 - Completion: Fix enter key deleting text after closing pairs (fixes #3191)
- #3197 - Vim: Fix hang when using the `experimental.viml` setting (fixes #3196 - thanks @amiralies!)
- #3205 - Editor: Update highlights and diagnostics immediately on buffer update (fixes #2620, #1459)
- #3207 - Status Bar: Fix ghost text in some themes with transparent statusbar colors
- #3217 - CLI: Add -v version flag (fixes #3209)
- #3225 - Extension - C#: Fix language server not starting on Windows (fixes #3204)
- #3226 - Keybindings: Fix issue opening keybindings.json from the command palette
- #3227 - Configuration: Allow string numbers as font sizes
- #3230 - Font: Treat "FiraCode-Regular.ttf" as default font
- #3233 - Formatting: Fix buffer de-sync when applying formatting edits (fixes #2196, #2820)
- #3239 - Buffers: Fix filetype picker not working as expected without an active workspace
- #3240 - Formatting: Fix 'Invalid Range Specified' error (fixes #3014)
- #3241 - Extensions: Handle `maxCount` and FS errors in `vscode.workspace.findFiles` (related #3215)
- #3248 - Extension - Windows: Fix path normalization issue in document selector (fixes #3238)
- #3251 - QuickOpen: Show filename first in Control+P/Command+P menu (fixes #2259, #3165)
- #3249 - Extensions: Send 'onCommand' activation event (related #3215)
- #3252 - UX: Remove glitchy pane animation (fixes #3245)
- #3255 - CodeLens: Fix disappearing lens when pressing enter in insert mode
- #3259 - Quickmenu: Implement smart case (thanks @amiralies!)
- #3264 - Formatting: Add 'Format Document' to menu
- #3273 - Completion: Use the default insert/replace range when provided (fixes #2388)
- #3274 - Completion: Fix race condition between completion subscription and buffer updates (fixes #3274)
- #3263 - Formatting: Update cursor position based on formatting edits
- #3272 - Snippets: Fix error parsing '@media' sass snippet
- #3276 - Completion: Handle replace range after cursor position (related #2583)
- #3280 - Quickmenu: Fix delay in processing Control+W key (fixes #3262)
- #3279 - Completion: Fix issue completing ReScript identifiers (fixes #3258)
- #3286 - Quickmenu: Scope control+tab to visible editor (fixes #3275, #2009)

### Performance

- #3148 - Large Files: Improve performance & fix crash when opening large files (related #1670)
- #3139 - Large Files: Fix hang when using `/` search (fixes #1670)
- #3260 - Editor: Compute minimal updates, fix flash when reloading file

### Documentation

- #3181 - Snippets: Initial snippets section
- #3185 - Emmet: Initial emmet section
- #3223 - Languages: Split out ReScript into separate section
- #3237 - Languages: Remove exuberant ctags from C/C++ section

### Infrastructure / Refactoring

- #3156 - Dependency: reason-native (dir/fp/fs) -> e16590c
- #3164, #3171, #3177, #3179, #3188 - Configuration: Move legacy configuration parsers to new model
- #3169 - Extensions: Remove some unused static assets from One Dark Pro
- #3168 - Diagnostics: Add additional build-information logging
- #3172 - Theme: Move theme loader to subscription (unblocks #3160)
- #3178 - Commands: Remove unused contributedCommands argument
- #3202 - Dependency: revery -> 2a59280
- #3220 - Dependency: revery -> 26e8b73 (Unblock OCaml 4.11)
- #3228 - Quickmenu: Initial feature implementation
- #3250 - Quickmenu: Move theme menus to new Quickmenu feature
- #3254 - Release: Fix notarization step

# 0.5.4 (March 2021)

### Features 

- #3024 - Snippet Support - Multi-select handler
- #3047, #3052, #3056, #3059, #3061, #3064, #3105 - Snippets: Core Feature Work
- #3067 - Snippets: Integrate snippets provided by extensions
- #3090 - Snippets: Add insert snippet command
- #3105 - Snippets: Implement configuration setting
- #3132 - Snippets: User snippet editing
- #2244 - Extension: Emmet support (fixes #1948)

### Bug Fixes

- #3008 - SCM: Fix index-out-of-bound exception when rendering diff markers
- #3007 - Extensions: Show 'missing dependency' activation error to user
- #3011 - Vim: Visual Block - Handle 'I' and 'A' in visual block mode (fixes #1633)
- #3016 - Extensions: Fix memory leak in extension host language features (fixes #3009)
- #3019 - Extensions: Fix activation error for Ionide.Ionide-fsharp extension (fixes #2974)
- #3020 - Vim: Fix incsearch cursor movement (fixes #2968)
- #3023 - Keybindings: Fix default - add quotes around "when" (thanks @noahhaasis!)
- #3027 - Vim: Command-line completion - fix 'set no' completion
- #3029 - Editor: Fix rubber-banding while scrolling with high key-repeat set
- #3021 - Configuration: Fix zoom being reset when saving configuration (fixes #2294)
- #3051 - Editor: Make horizontal / vertical scrollbars on editor surface configurable (fixes #3036)
- #3030 - Extensions: Implement workspace storage (related #2676)
- #3052 - Input: Fix key being 'eaten' after executing remapped key (fixes #3048)
- #3060 - Snippets: Fix parser handling of stand-alone curly braces
- #3044 - Search: Add `search.exclude` configuration option (fixes #2115 - thanks @joseeMDS!)
- #3066 - Vim / Input: Fix ':map' condition (fixes #3049)
- #3055, #3088 - Extensions: Implement 'vscode.openFolder' handler (related #3042)
- #3076 - Terminal: Add `ONIVIM_TERMINAL` environment variable (fixes #3068)
- #3078 - Auto-Update: Notify user when update fails due to missing key (fixes #3070)
- #3086 - Snippets: Fix clash with completion / document highlights feature
- #3085 - Snippets: Fix drop-shadow calculation at end of buffer
- #3091 - Snippets: Fix auto-closing pairs when placeholders are on same line
- #3102 - Vim / Input: Implement mapping timeout (fixes #2850)
- #3121 - Snippets: Add support for the $TM_SELECTED_TEXT snippet variable
- #3122 - Signature Help: Close signature help when traversing snippet placeholders
- #3123 - Extensions: Fix failure to install extensions over 10MB from open-vsx
- #3054 - Extensions: Completion - Implement 'isIncomplete' handler (fixes #3022, #2359)
- #3129 - Snippets: Replace visual/select range on insert
- #3057 - Theming: Turn down shadow intensity for light themes (related #3095)
- #3133 - Completion: Implement shift+escape to close all popups w/o switching modes (fixes #3120)
- #3134 - Snippets: Only show snippet visualizer for active editor
- #3135 - Snippets: Convert choices to placeholders
- #3137 - Snippets: Fix error parsing some snippets in the React TS/JS extensions

### Performance

- #3116 - SCM: Recalculate diff markers less frequently (only on buffer updates)

### Infrastructure / Refactoring

- #3101 - Dependency: reason-fzy -> 485cae1
- #3096 - OS: Add logging for `readdir` path (related #3092)
- #3097 - Configuration: Remove unused `workbench.tree.indent` setting
- #3115 - Dependency: vscode-exthost -> 1.53.0

## 0.5.3 (Feb 2021)

### Features 

- #2866 - Extensions: Show ratings / download count in details view (fixes #2866)
- #2889 - Extensions: Include HTML, JSON, PHP, and Markdown language servers
- #2849 - UX: Initial menu bar on Windows / Linux (fixes #1255)
- #2596 - Editor: File preview when clicking on files (thanks @fanantoxa!)
- #2755 - Editor: Implement customizable font-weight (fixes #1573, thanks @marcagba!)
- #2940 - Extensions: CodeLens - promote from experimental to on-by-default

> NOTE: CodeLens can be disabled with either `"editor.codeLens": false` or `:set nocodelens`.

- #2969 - UX: Menu bar integration on OSX

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
- #2938 - Input: OSX - Modifier keys not working on Romaji keyboard (fixes #2924)
- #2941 - Input: Fix handling of `<space>` as leader key (fixes #2935)
- #2944 - Components: Remove overscroll in `VimList`
- #2942 - TextMate: Fix infinite loop with vala grammar (fixes #2933)
- #2937 - Editor: Fix bugs around horizontal scrolling (fixes #1544, #2914)
- #2946 - SCM: Fix StackOverflow when retrieving original content for large files
- #2954 - Markdown: explicitly set code block font size (fixes #2953)
- #2950 - Input: Fix binding to `+` key (fixes #2293)
- #2955 - UX: Fix hardcoded theme colors in extensions list/details
- #2898, #2966 - Editor: Implement mouse selection (fixes #537)
- #2898 - Editor: Implement mouse selection (fixes #537)
- #2959 - Completion: Implement `"editor.acceptSuggestionOnEnter"` configuration setting
- #2956 - Layout: Fix extra editor when splitting with a file or terminal (fixes #2900, #2952)
- #2986 - Theme: Fallback to default theme if invalid theme is specified
- #2977, #2983 - Input: Handle unicode characters in mappings (fixes #2972, #2980)
- #2984 - Editor: Fix extraneous clones of editor on Control+Tab (fixes #2988)
- #2978 - Input: Fix `m-` modifier behavior (fixes #2963)
- #2990 - Signature Help: Fix blocking `esc` key press back to normal mode
- #2991 - OSX: Fix shortcut keys double-triggering events
- #2993 - CodeLens: Handle null command id & label icons
- #2997 - Syntax: Fix regression in syntax highlighting for PHP (fixes #2985)
- #2995 - Extensions: Fix bug with 3-param http/https request (fixes #2981)
- #2999 - Extensions: Elm - fix bug with diagnostics not displaying (fixes #2640)
- #3000 - Extensions: Search - Fix out-of-order search results (fixes #2979 - thanks @jakubbaron!)
- #3003 - Extensions: CodeLens - Handle the `$emitCodeLens` event

### Performance

- #2852, #2864 - Performance: Batch editor / codelens animations
- #2901 - Bundle Size: Remove unused Selawik and Inconsolata fonts
- #2932 - Language Features: Debounce high-frequency subscriptions

### Documentation

- #2874 - Languages: Fix extension ids for `exuberant-ctags` and `clangd` (fixes #2872)
- #2893 - Building: Add Xorg dependencies for Linux (thanks @marcinkoziej!)
- #2939 - Release: Document monthly release process
- #2996 - Building: Reorder instructions so that `git clone` is first step (thanks @jakubbaron!)

### Infrastructure / Refactoring

- #2853 - Input: Add APIs for querying contextually available bindings and consumed keys
- #2886 - Extensions: Upgrade vscode-exthost -> 1.51.0
- #2887 - Build: Remove hardcoded extension host version; pull from package
- #2888 - Extensions: Upgrade vscode-exthost -> 1.52.1
- #2894 - Build: Linux - Fix permission problem removing setup.json (thanks @marcinkoziej!)
- #2889 - Extensions: Upgrade extensions to 1.52.1
- #2904 - Build: Fix warnings in macOS build (thanks @zbaylin!)
- #2964 - Dependency: Upgrade revery -> 3f48f6d
- #2975 - Dependency: Upgrade revery -> 7191349
- #2992 - Dependency: Upgrade revery -> 8497f52

## 0.5.2 (Jan 2021)

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
- #2833 - Outline: Don't wrap long signatures when overflowing (fixes #2626)

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
- #2841 - UX: Tone down shadow color for light themes
- #2840 - Extensions: CodeLens - handle toggling via `:set codelens`

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

## 0.5.1 (Dec 2020)

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
