# CHANGELOG
## 2.18.2
- Update README.MD

## 2.18.1
- Add OneDark-Pro-bold style

## 2.17.7
- Fix border in dropdown appearing without focus

## 2.17.6 | 2018.10.25
- Change focus from `#F8FAFD` to `#464646` to dim the colour

## 2.17.5
- Change focus from `#21252b` to `#F8FAFD` to improve accessibility in GUIs

## 2.17.4 | 2018.10.14
- Improve markdown support
- Improve editor dropdown color

## 2.17.3 | 2018.10.05
- Improve elixir support #262
- Improve flow support #222

## 2.17.1 | 2018.10.04
- Rewrite syntax highlight file, simplified color, update docs

## 2.16.7 | 2018.10.04
Imporve Ruby and FlowJS support

## 2.16.6 | 2018.09.12

- Edit editor tab highlighting to be the `#282c34` colour to be consistent across the theme
- Improve Python italics support

## 2.16.3 | 2018.9.09

- Change: Separate editor styles and syntax highlighting

## 2.16.1 | 2018.9.06

- Made units red again
- Adjusted colours of operators (+ compound operators) to be consistent, such as `++`, `--` and `*=` etc.
- Improve Punctuation (some)
- Improve some syntax highlighting for languages:
  - Clojure (globals, symbols and constants)
  - CSS/SCSS/LESS (Colour Operators such as \* / + -)
  - CoffeeScript (Brung some colours closer to JavaScript, for consistency)
  - Ini (Highlighted default text so it's visible as values rather than plain text)
  - Go (Package Name highlighting)
  - Groovy (Better function and variable highlighting)
  - HLSL (More keywords, semantics highlighted)
  - Makefile (Prerequisities highlighting, text colour treated as values inputted, highlighted)
  - Markdown (Better Lists and Links and Image highlighting)
  - R (Correct Function highlighting)
  - SQL (Variables Highlighting, Bracketed Text Highlighting)
  - Swift (Type Highlighting)
  - Visual Basic (Type Highlighting)

## 2.15.4 | 2018.8.09

- Change C/C++ comments from bold to italic to make more unified #242

## 2.15.3 | 2018.7.31

- Change C/C++ comments from italic to bold
- Fix php Static class variable highlight error
- Fix C/C++ right shift highlight error

## 2.15.1 | 2018.7.05

- Fix css color highlight error
- Revert changes to fix YAML unquoted strings

## 2.15.0 | 2018.7.02

- Made units (px, em.. etc) red to distinguish between the value itself and the units: easier readability
- Made pseudo-elements/classes bluish in order to distinguish it from classes (which are the same colour in css/less/scss)
- Made escaped characters (such as &apos; &copy; etc) red to see them easily, especially in Pug (jade) files where brackets and classes are essentially the same colour
- The String literal in JS/TS ${} is now purply for easier readability
- All Comments are now italic
- Changes to TypeScript/JavaScript (possibly other languages too) syntax highlighting:
  - Classes now all Yellow (except in TS in the case of: App.getSmth(), as both normal objects and classes use the same code inetrnally, meaning highlighting would be weird)
  - Property names in `js let app = { hello: "world"}` now red, to match selector (eg: `js app.hello`) and themselves
  - Optional operator `ts ?:` now purple too

## 2.14.0 | 2018.6.14

- Change terminal ansi color to match syntax color
- Improve tab

## 2.13.6 | 2018.5.17

- Add Contributors

## 2.13.5 | 2018.5.04

- Improve go&elm highlight
- Support Highlighted indent guides
- Hide blue border around file explorer when focused

## 2.13.4 | 2018.3.12

- Updated `.vsixmanifest` to contain latest and correct data.
- Updated `LICENSE` copyright date.
- Deleted `vsc-extension-quickstart.md` which was auto-generated.
- Updated `name` in themes to match correct theme name.
- Minor changes in `docs/index.html`.
- Minor changes in `README.md` and `docs/*.md` files.
- Added `galleryBanner` in `package.json` to match theme colors.
- Revert back original terminal coloring like it was before.

## 2.13.3 | 2018.3.11

- Updated cursor color. [#189](https://github.com/Binaryify/OneDark-Pro/issues/189)
- Removed all terminal colors. [#195](https://github.com/Binaryify/OneDark-Pro/issues/195)

## 2.13.2 | 2018.3.11

- Updated color of current active line and active line number in the editor.

## 2.13.1 | 2018.3.10

- Updated border colors for editor highlights.

## 2.13.0 | 2018.3.08

- Fixed `findMatchBackground` coloring.
- New color for line number of the current active line in the editor.
- Deleted old `notification.background` color because of the new Notification Center.
- New border colors for editor highlights which improves readability while searching.
- Add some edgeSyntax syntax support

## 2.12.8 | 2018.3.06

Improve integrated terminal colors

## 2.12.7 | 2018.3.05

Update integrated terminal colors with One Dark colors

## 2.12.6 | 2018.2.28

String Interpolation for visual grepping fixes #191

## 2.12.5 | 2018.2.24

Improve bracket visibility

Improve line highlight background and scrollbar slider background

Fix weird php behavior when a interface extends a PHP interface
thanks for @nicovak @jens1o @svipben

## 2.12.4 | 2018.2.11

Improve React component tags's hightlight color

## 2.12.3 | 2018.2.08

Improve colors for Editor Tabs

## 2.12.2 | 2018.1.16

improve cursor color

## 2.12.0 | 2018.1.15

improve cursor color & selection background color

## 2.11.1 | 2018.1.1

import C# highlight color
[#169 @ritwickdey](https://github.com/Binaryify/OneDark-Pro/issues/169) Happy
New Year~

## 2.11.0 | 2017.12.14

Improve status bar debugging color

## 2.10.23 | 2017.11.25

Fix `use Test\ClassNameA as ClassNameAAliased;` in php

## 2.10.22 | 2017.11.17

Improve markdown inline code highlight

## 2.10.21 | 2017.11.16

Improve markdown inline code highlight

## 2.10.20 | 2017.11.10

Fix some php highlight

## 2.10.19 | 2017.11.09

Improve terminal & regexp color

## 2.10.18 | 2017.11.05

Improve terminal color

## 2.10.17 | 2017.10.17

Fix a typo in comment color code

## 2.10.15 | 2017.10.07

Improve comment highlight

## 2.10.14 | 2017.10.06

Update docs

## 2.10.13 | 2017.10.05

Changed comment to be brighter

## 2.10.12 | 2017.9.23

Add TS primitive/builtin types support in TSX

## 2.10.11 | 2017.9.6

fix Python string color inconsistency issue
[@jens1o](https://github.com/jens1o)\
[PR:142](https://github.com/Binaryify/OneDark-Pro/pull/142)

## 2.10.10 | 2017.8.24

Reset more styles [@jens1o](https://github.com/jens1o)\
[PR:139](https://github.com/Binaryify/OneDark-Pro/pull/139)

## 2.10.9 | 2017.8.21

Improve diff highlight

## 2.10.8 | 2017.8.12

Improve java highlight

## 2.10.7 | 2017.8.2

Add php screenshot

## 2.10.6 | 2017.7.31

Make JS Math constants to be in sync with Atom, fix (S)CSS selectors font style
error

## 2.10.3 | 2017.7.29

Fix php array() syntax, fix the vivid theme file path error

## 2.10.0 | 2017.7.28

Support basic markdown syntax when using vscode-todo, fix generic types in
phpdoc, improve JavaScript constants color, add vivid theme

## 2.9.8 | 2017.7.22

Support php types in documentations

## 2.9.7 | 2017.7.19

Improve selected variable highlight
contributor:[@bardiarastin](https://github.com/bardiarastin) on issue
[#119](https://github.com/Binaryify/OneDark-Pro/issues/119)

## 2.9.5 | 2017.7.18

Increase the contrast of the selected text

## 2.9.4 | 2017.7.15

Reset php round bracket in method parameters

## 2.9.3 | 2017.7.12

reset php function call labels

## 2.9.2 | 2017.7.11

Improve js/ts keyword 'import' highlight

## 2.9.1 | 2017.7.1

Improve php support, fix icon format error

## 2.8.9 | 2017.6.22

Improve badge background & background color of 'Go to next error or warning'
(Keyboard shortcut: F8) popup, add support.variable.property.process &
debugToolBar background & support.variable.object.node support

## 2.8.8 | 2017.6.20

Improve ruler color, improve comment fontStyle, improve peek implementation
match highlight background color

## 2.8.7 | 2017.6.19

Add jsx/tsx support, improve color of warning/error squiggles, improve ruler
color

## 2.8.6 | 2017.6.13

Support js/ts for-of operator, support php nowdoc, support void keyword, add
bold and italics support for Markdown

## 2.8.5 | 2017.6.6

Fix js/ts key-value separator highlight error

## 2.8.4 | 2017.6.2

Rollback for the issue [#87](https://github.com/Binaryify/OneDark-Pro/issues/87)

## 2.8.3 | 2017.6.2

improve JS/TS object-literal key highlight

## 2.8.2 | 2017.6.1

improve C highlight

## 2.8.1 | 2017.5.29

support void keyword, support ternary operators, support bitwise operators,
support is expression in typescript

## 2.8.0 | 2017.5.22

resolve issue #48 #65 #66 #69 #70, support php comparison, add support for
arithmetic operators, add support for php regex operators, support heredoc php
operator

## 2.7.8 | 2017.5.17

improve block cursor contrast background color

## 2.7.7 | 2017.5.16

Fix this expand selection error, modified/added activityBar colors, added
scrollBar colors

## 2.7.5 | 2017.5.14

Support php-parser constants, improve activity bar badge color, fix this expand
selection error

## 2.7.4 | 2017.5.11

Few more workbench colour fixes, Update diffEditor.insertedTextBackground

## 2.7.3 | 2017.5.10

New logo and window colour additions and tweaks
[https://github.com/Binaryify/OneDark-Pro/pull/51](https://github.com/Binaryify/OneDark-Pro/pull/51)

## 2.7.2 | 2017.5.9

add support for php's goto, update docs

## 2.7.1 | 2017.5.7

added custom colors for sidebar, statusBar, list, input boxes, etc.

## 2.7.0 | 2017.5.5

Change theme file format. Add workbench support, improve ts support, update
document

## 2.6.3 | 2017.5.2

Make displayName and label be the same to avoid confusion, for the reason
[https://github.com/Binaryify/OneDark-Pro/issues/41](https://github.com/Binaryify/OneDark-Pro/issues/41)

## 2.6.0 | 2017.5.1

rename to "OneDark Pro" for the reason
[https://github.com/Binaryify/OneDark-Pro/issues/40](https://github.com/Binaryify/OneDark-Pro/issues/40)

## 2.5.3 | 2017.4.24

fix C++ heightlight error

## 2.5.2 | 2017.4.21

added support support for other class namespace outside use

## 2.5.0 | 2017.4.15

Added php function-call object and static

## 2.4.16 | 2017.4.12

improve php & C# SUPPORT

## 2.4.15 | 2017.4.10

support php constants and normalize array in double quoted strings,add keywords,
rename

## 2.4.6 | 2017.4.9

add DOCS

## 2.4.4 | 2017.4.9

improve php support

## 2.4.3 | 2017.4.8

improve python types support & C++ support

## 2.4.1 | 2017.4.8

improve python types support

## 2.4.0 | 2017.4.7

improve python logical & variable parameter support

## 2.3.9 | 2017.4.7

improve python support

## 2.3.8 | 2017.4.7

improve document

## 2.3.7 | 2017.4.7

improve C++ support

## 2.3.6 | 2017.4.6

improve php dollar sign & logical operator height light

## 2.3.5 | 2017.3.31

improve JavaScript 'instanceof' & 'process' height light

## 2.3.4 | 2017.3.29

improve Java height light

## 2.3.3

add php call-function height light

## 2.3.2

update icon

## 2.3.1

improve js module height light

## 2.3.0

improve README.MD

## 2.2.9

add css/scss property value hight light

## 2.2.8

improve js 'this' height light

## 2.2.7

improve js variable property, js object-literal key, constant.js and math height
light,remove notice

## 2.2.6

add README.MD notice

## 2.2.5

remove js operator accessor hight light

## 2.2.4

push changelog to CHANGELOG.md

## 2.2.3

improve css's id & class & color name height light

## 2.2.2

improve README.MD, add changelog

## 2.2.1

add js module hight light, add changelog

## 2.2.0

add js operator logical hight light

## 2.1.9

add js console height light

## 2.1.8

add java support

## 2.1.7

fix diff height light error

## 2.1.6

improve js operator
