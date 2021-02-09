### Features 

- #3024 - Snippet Support - Multi-select handler
- #3047, #3052, #3056, #3059, #3061, #3064 - Snippets: Core Feature Work
- #3067 - Snippets: Integrate snippets provided by extensions
- #3090 - Snippets: Add insert snippet command
- #3105 - Snippets: Implement configuration setting

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

### Performance

- #3116 - SCM: Recalculate diff markers less frequently (only on buffer updates)

### Documentation

### Infrastructure / Refactoring

- #3012 - Node: Upgrade from Node 12 LTS -> Node 14 LTS (related #3009)
- #3101 - Dependency: reason-fzy -> 485cae1
- #3096 - OS: Add logging for `readdir` path (related #3092)
- #3097 - Configuration: Remove unused `workbench.tree.indent` setting
