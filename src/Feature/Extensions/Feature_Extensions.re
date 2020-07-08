open Oni_Core;
open Exthost.Extension;

include Model;

type outmsg =
  | Nothing
  | Focus
  | Effect(Isolinear.Effect.t(msg));

module Internal = {
  let markActivated = (id: string, model) => {
    ...model,
    activatedIds: [id, ...model.activatedIds],
  };

  let add = (extensions, model) => {
    ...model,
    extensions: extensions @ model.extensions,
  };
};

let checkAndUpdateSearchText = (~previousText, ~newText, ~query) =>
  if (previousText != newText) {
    if (String.length(newText) == 0) {
      None;
    } else {
      Some(Service_Extensions.Query.create(~searchText=newText));
    };
  } else {
    query;
  };

let update = (~extHostClient, msg, model) => {
  switch (msg) {
  | Activated(id) => (Internal.markActivated(id, model), Nothing)
  | Discovered(extensions) => (Internal.add(extensions, model), Nothing)
  | ExecuteCommand({command, arguments}) => (
      model,
      Effect(
        Service_Exthost.Effects.Commands.executeContributedCommand(
          ~command,
          ~arguments,
          extHostClient,
        ),
      ),
    )
  | KeyPressed(key) =>
    let previousText = model.searchText |> Feature_InputText.value;
    let searchText' = Feature_InputText.handleInput(~key, model.searchText);
    let newText = searchText' |> Feature_InputText.value;
    let latestQuery =
      checkAndUpdateSearchText(
        ~previousText,
        ~newText,
        ~query=model.latestQuery,
      );
    ({...model, searchText: searchText', latestQuery}, Nothing);
  | SearchText(msg) =>
    let previousText = model.searchText |> Feature_InputText.value;
    let searchText' = Feature_InputText.update(msg, model.searchText);
    let newText = searchText' |> Feature_InputText.value;
    let latestQuery =
      checkAndUpdateSearchText(
        ~previousText,
        ~newText,
        ~query=model.latestQuery,
      );
    ({...model, searchText: searchText', latestQuery}, Focus);
  | SearchQueryResults(queryResults) => (
      {...model, latestQuery: Some(queryResults)},
      Nothing,
    )
  | SearchQueryError(_queryResults) =>
    // TODO: Error experience?
    ({...model, latestQuery: None}, Nothing)
  };
};

let all = ({extensions, _}) => extensions;
let activatedIds = ({activatedIds, _}) => activatedIds;

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

module ListView = ListView;

let sub = (~setup, model) => {
  let toMsg =
    fun
    | Ok(query) => SearchQueryResults(query)
    | Error(err) => SearchQueryError(err);

  switch (model.latestQuery) {
  | Some(query) when !Service_Extensions.Query.isComplete(query) =>
    Service_Extensions.Sub.search(~setup, ~query, ~toMsg)
  | Some(_)
  | None => Isolinear.Sub.none
  };
};
