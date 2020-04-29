/*
 * Extensions.re
 *
 * This module models state around loaded / activated extensions
 * for the 'Hover' view
 */
open Oni_Core;

open Exthost.Extension;

type t = {
  activatedIds: list(string),
  extensions: list(Scanner.ScanResult.t),
};

[@deriving show({with_path: false})]
type action =
  | Activated(string /* id */)
  | Discovered([@opaque] list(Scanner.ScanResult.t))
  | ExecuteCommand({
      command: string,
      arguments: [@opaque] list(Json.t),
    });

let empty = {activatedIds: [], extensions: []};

let markActivated = (id: string, model: t) => {
  ...model,
  activatedIds: [id, ...model.activatedIds],
};

let add = (extensions, model) => {
  ...model,
  extensions: extensions @ model.extensions,
};

let _filterBundled = (scanner: Scanner.ScanResult.t) => {
  let name = scanner.manifest.name;

  name == "vscode.typescript-language-features"
  || name == "vscode.markdown-language-features"
  || name == "vscode.css-language-features"
  || name == "vscode.html-language-features"
  || name == "vscode.laserwave"
  || name == "vscode.Material-theme"
  || name == "vscode.reason-vscode"
  || name == "vscode.gruvbox"
  || name == "vscode.nord-visual-studio-code";
};

let getExtensions = (~category, model) => {
  let results =
    model.extensions
    |> List.filter((ext: Scanner.ScanResult.t) => ext.category == category);

  switch (category) {
  | Scanner.Bundled => List.filter(_filterBundled, results)
  | _ => results
  };
};

// TODO: Should be stored as proper commands instead of converting every time
let commands = model => {
  model.extensions
  |> List.map((ext: Scanner.ScanResult.t) =>
       ext.manifest.contributes.commands
     )
  |> List.flatten
  |> List.map((extcmd: Contributions.Command.t) =>
       Command.{
         id: extcmd.command,
         category: extcmd.category,
         title: Some(extcmd.title |> LocalizedToken.toString),
         icon: None,
         isEnabledWhen: extcmd.condition,
         msg:
           `Arg1(
             arg =>
               ExecuteCommand({command: extcmd.command, arguments: [arg]}),
           ),
       }
     );
};

let menus = model =>
  // Combine menu items contributed to common menus from different extensions
  List.fold_left(
    (acc, extension: Scanner.ScanResult.t) =>
      List.fold_left(
        (acc, menu: Menu.Schema.definition) =>
          StringMap.add(menu.id, menu.items, acc),
        StringMap.empty,
        extension.manifest.contributes.menus,
      )
      |> StringMap.union((_, xs, ys) => Some(xs @ ys), acc),
    StringMap.empty,
    model.extensions,
  )
  |> StringMap.to_seq
  |> Seq.map(((id, items)) => Menu.Schema.{id, items})
  |> List.of_seq;
