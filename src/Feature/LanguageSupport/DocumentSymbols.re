open EditorCoreTypes;
open Oni_Core;

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

[@deriving show]
type symbol = {
  uniqueId: string,
  name: string,
  detail: string,
  kind: Exthost.SymbolKind.t,
  range: CharacterRange.t,
  selectionRange: CharacterRange.t,
};

let sortSymbolsByPosition = (a: symbol, b: symbol) => {
  CharacterRange.compare(a.range, b.range);
};

type t = list(Tree.t(symbol, symbol));

let extHostSymbolToTree: Exthost.DocumentSymbol.t => Tree.t(symbol, symbol) =
  symbol => {
    let rec loop = (~uniqueIdPrefix, extSymbol: Exthost.DocumentSymbol.t) => {
      let newUniqueId = uniqueIdPrefix ++ "." ++ extSymbol.name;
      let children =
        extSymbol.children |> List.rev_map(loop(~uniqueIdPrefix=newUniqueId));

      let symbol = {
        uniqueId: newUniqueId,
        name: extSymbol.name,
        detail: extSymbol.detail,
        kind: extSymbol.kind,
        range: extSymbol.range |> Exthost.OneBasedRange.toRange,
        selectionRange: extSymbol.range |> Exthost.OneBasedRange.toRange,
      };

      if (children == []) {
        Tree.leaf(symbol);
      } else {
        Tree.node(~children, symbol);
      };
    };

    loop(~uniqueIdPrefix="symbol.", symbol);
  };

type model = {
  providers: list(provider),
  bufferToSymbols: IntMap.t(list(Tree.t(symbol, symbol))),
};

let initial = {providers: [], bufferToSymbols: IntMap.empty};

[@deriving show]
type command =
  | GotoSymbol;

[@deriving show]
type quickmenu =
  | NoSymbolSelected
  | SymbolSelected({
      filePath: string,
      symbol,
    });

[@deriving show]
type msg =
  | Command(command)
  | Quickmenu(quickmenu)
  | DocumentSymbolsAvailable({
      bufferId: int,
      symbols: list(Exthost.DocumentSymbol.t),
    });

let get = (~bufferId, model) => {
  IntMap.find_opt(bufferId, model.bufferToSymbols);
};

let update = (~maybeBuffer, msg, model) => {
  switch (msg) {
  | Command(GotoSymbol) =>
    let maybeFilePath =
      maybeBuffer |> Utility.OptionEx.flatMap(Buffer.getFilePath);
    let outmsg =
      maybeBuffer
      |> Option.map(Oni_Core.Buffer.getId)
      |> Utility.OptionEx.flatMap(bufferId => get(~bufferId, model))
      |> Utility.OptionEx.map2(
           (filePath, symbols) => {
             let allItems =
               symbols
               |> List.map(TreeList.ofTree)
               |> List.flatten
               |> List.map(
                    TreeList.(
                      fun
                      | ViewLeaf({data, _}) => data
                      | ViewNode({data, _}) => data
                    ),
                  )
               |> List.sort(sortSymbolsByPosition);

             let onAccepted = (~text as _, ~item as maybeSymbol) => {
               maybeSymbol
               |> Option.map(symbol =>
                    Quickmenu(SymbolSelected({filePath, symbol}))
                  )
               |> Option.value(~default=Quickmenu(NoSymbolSelected));
             };

             let itemToIcon = ({kind, _}: symbol) => {
               let icon =
                 Oni_Components.SymbolIcon.Internal.symbolToIcon(kind);
               let color =
                 Oni_Components.SymbolIcon.Internal.symbolToColor(kind);
               Some(Feature_Quickmenu.Schema.Icon.codicon(~color, icon));
             };
             let itemRenderer =
               Feature_Quickmenu.Schema.Renderer.defaultWithIcon(itemToIcon);
             Outmsg.ShowMenu(
               Feature_Quickmenu.Schema.menu(
                 ~focusFirstItemByDefault=true,
                 ~onAccepted,
                 ~toString=(symbol: symbol) => symbol.name,
                 ~itemRenderer,
                 allItems,
               ),
             );
           },
           maybeFilePath,
         )
      |> Option.value(~default=Outmsg.Nothing);

    (model, outmsg);
  | DocumentSymbolsAvailable({bufferId, symbols}) =>
    let symbolTrees = symbols |> List.rev_map(extHostSymbolToTree);
    let bufferToSymbols =
      IntMap.add(bufferId, symbolTrees, model.bufferToSymbols);
    ({...model, bufferToSymbols}, Outmsg.Nothing);

  | Quickmenu(SymbolSelected({filePath, symbol})) => (
      model,
      Outmsg.OpenFile({
        filePath,
        location: Some(symbol.range.start),
        direction: SplitDirection.Current,
      }),
    )

  | Quickmenu(NoSymbolSelected) => (model, Outmsg.Nothing)
  };
};

let register = (~handle: int, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

let sub = (~buffer, ~client, model) => {
  let toMsg = symbols => {
    DocumentSymbolsAvailable({
      bufferId: Oni_Core.Buffer.getId(buffer),
      symbols,
    });
  };
  //
  model.providers
  |> List.filter(({selector, _}) =>
       selector |> Exthost.DocumentSelector.matchesBuffer(~buffer)
     )
  |> List.map(({handle, _}) => {
       Service_Exthost.Sub.documentSymbols(~handle, ~buffer, ~toMsg, client)
     })
  |> Isolinear.Sub.batch;
};

module Commands = {
  open Feature_Commands.Schema;

  let gotoSymbol =
    define(
      ~category="Language Support",
      ~title="Go to buffer symbol",
      "workbench.action.gotoSymbol",
      Command(GotoSymbol),
    );
};

module Keybindings = {
  open Feature_Input.Schema;

  let condition = "editorTextFocus && normalMode" |> WhenExpr.parse;

  let gotoSymbol =
    bind(~key="gs", ~command=Commands.gotoSymbol.id, ~condition);
};

module MenuItems = {
  open ContextMenu.Schema;

  let gotoBufferSymbol =
    command(~title="Go to Symbol in Buffer...", Commands.gotoSymbol);
};

module Contributions = {
  let commands = Commands.[gotoSymbol];

  let keybindings = Keybindings.[gotoSymbol];

  let menuGroups =
    ContextMenu.Schema.[
      group(
        ~order=200,
        ~parent=Feature_MenuBar.Global.go,
        MenuItems.[gotoBufferSymbol],
      ),
    ];
};
