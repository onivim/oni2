open Exthost.Types;

let explorerContext: Menu.Lookup.t => list(Menu.item);
let editorContext: Menu.Lookup.t => list(Menu.item);
let editorTitle: Menu.Lookup.t => list(Menu.item);
let editorTitleContext: Menu.Lookup.t => list(Menu.item);
let debugCallstackContext: Menu.Lookup.t => list(Menu.item);
let debugToolbar: Menu.Lookup.t => list(Menu.item);
let scmTitle: Menu.Lookup.t => list(Menu.item);
let scmResourceGroupContext: Menu.Lookup.t => list(Menu.item);
let scmResourceStateContext: Menu.Lookup.t => list(Menu.item);
let scmChangeTitle: Menu.Lookup.t => list(Menu.item);
let viewTitle: Menu.Lookup.t => list(Menu.item);
let viewItemContext: Menu.Lookup.t => list(Menu.item);
let touchBar: Menu.Lookup.t => list(Menu.item);
let commentThreadTitle: Menu.Lookup.t => list(Menu.item);
let commentThreadContext: Menu.Lookup.t => list(Menu.item);
let commentTitle: Menu.Lookup.t => list(Menu.item);
let commentContext: Menu.Lookup.t => list(Menu.item);

let commandPalette:
  (WhenExpr.ContextKeys.t, Command.Lookup.t(_), Menu.Lookup.t) =>
  list(Menu.item);
