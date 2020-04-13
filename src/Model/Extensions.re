/*
 * Extensions.re
 *
 * This module models state around loaded / activated extensions
 * for the 'Hover' view
 */
open Oni_Core;
open Oni_Extensions;

type t = {
  activatedIds: list(string),
  extensions: list(ExtensionScanner.t),
};

[@deriving show({with_path: false})]
type action =
  | Activated(string /* id */)
  | Discovered([@opaque] list(ExtensionScanner.t))
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

let _filterBundled = (scanner: ExtensionScanner.t) => {
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
    |> List.filter((ext: ExtensionScanner.t) => ext.category == category);

  switch (category) {
  | ExtensionScanner.Bundled => List.filter(_filterBundled, results)
  | _ => results
  };
};

// TODO: Should be stored as proper commands instead of converting every time
let commands = model => {
  model.extensions
  |> List.map((ext: ExtensionScanner.t) => ext.manifest.contributes.commands)
  |> List.flatten
  |> List.map((extcmd: ExtensionContributions.Command.t) =>
       Command.{
         id: extcmd.command,
         category: extcmd.category,
         title: Some(extcmd.title |> LocalizedToken.to_string),
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
    (acc, extension: ExtensionScanner.t) =>
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
