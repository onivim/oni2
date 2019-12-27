/*
 * Extensions.re
 *
 * This module models state around loaded / activated extensions
 * for the 'Hover' view
 */
open Oni_Extensions;

type t = {
  activatedIds: list(string),
  extensions: list(ExtensionScanner.t),
};

[@deriving show({with_path: false})]
type action =
  | Activated(string /* id */)
  | Discovered([@opaque] list(ExtensionScanner.t));

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
