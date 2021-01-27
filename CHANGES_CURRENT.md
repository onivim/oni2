### Features 

- #3024 - Editor: Snippet Support - Multi-select handler
- #3047, #3052 - Editor: Snippet Feature

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

### Performance

### Documentation

### Infrastructure / Refactoring

- #3012 - Node: Upgrade from Node 12 LTS -> Node 14 LTS (related #3009)
