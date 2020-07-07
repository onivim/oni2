open Oni_Core;
open Exthost.Extension;

[@deriving show({with_path: false})]
type msg =
  | Activated(string /* id */)
  | Discovered([@opaque] list(Scanner.ScanResult.t))
  | ExecuteCommand({
      command: string,
      arguments: [@opaque] list(Json.t),
    })
  | KeyPressed(string)
  | SearchQueryResults(Service_Extensions.Query.t)
  | SearchQueryError(string)
  | SearchText(Feature_InputText.msg);

type model = {
  activatedIds: list(string),
  extensions: list(Scanner.ScanResult.t),
  searchText: Feature_InputText.model,
};

let initial = {
  activatedIds: [],
  extensions: [],
  searchText: Feature_InputText.create(~placeholder="Type to search..."),
};

module Internal = {
  let filterBundled = (scanner: Scanner.ScanResult.t) => {
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
};

let getExtensions = (~category, model) => {
  let results =
    model.extensions
    |> List.filter((ext: Scanner.ScanResult.t) => ext.category == category);

  switch (category) {
  | Scanner.Bundled => List.filter(Internal.filterBundled, results)
  | _ => results
  };
};
