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
  latestQuery: option(Service_Extensions.Query.t),
};

let initial = {
  activatedIds: [],
  extensions: [],
  searchText: Feature_InputText.create(~placeholder="Type to search..."),
  latestQuery: None,
};

let searchResults = ({latestQuery, _}) =>
  switch (latestQuery) {
  | None => []
  | Some(query) => query |> Service_Extensions.Query.results
  };

module Internal = {
  let filterBundled = (scanner: Scanner.ScanResult.t) => {
    let name = scanner.manifest |> Manifest.identifier;

    name == "vscode.typescript-language-features"
    || name == "vscode.css-language-features"
    || name == "jaredkent.laserwave"
    || name == "jaredly.reason-vscode"
    || name == "arcticicestudio.nord-visual-studio-code";
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
