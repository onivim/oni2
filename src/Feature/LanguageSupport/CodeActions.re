open Oni_Core;
open Exthost;

type provider = {
  handle: int,
  selector: DocumentSelector.t,
  metadata: CodeAction.ProviderMetadata.t,
  displayName: string,
  supportsResolve: bool,
};

type model = {providers: list(provider)};

[@deriving show]
type msg = unit;

let initial = {providers: []};

let register =
    (~handle, ~selector, ~metadata, ~displayName, ~supportsResolve, model) => {
  {
    providers: [
      {handle, selector, metadata, displayName, supportsResolve},
      ...model.providers,
    ],
  };
};

let unregister = (~handle, model) => {
  providers:
    model.providers |> List.filter(provider => {provider.handle != handle}),
};

let update = (msg, model) => (model, Outmsg.Nothing);

let sub =
    (
      ~buffer,
      ~topVisibleBufferLine,
      ~bottomVisibleBufferLine,
      ~client,
      codeActions,
    ) => {
  // TODO:
  let context =
    Exthost.CodeAction.(Context.{only: None, trigger: TriggerType.Auto});

  let toMsg =
    fun
    | Ok(Some(actions)) =>
      prerr_endline(Exthost.CodeAction.List.toDebugString(actions))
    | Ok(None) => prerr_endline("NONE")
    | Error(msg) => prerr_endline("ERROR:" ++ msg);

  let lines =
    EditorCoreTypes.LineSpan.{
      start: topVisibleBufferLine,
      stop: bottomVisibleBufferLine,
    };

  codeActions.providers
  |> List.filter(({selector, _}) => {
       Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
     })
  |> List.map(({handle, _}) => {
       Service_Exthost.Sub.codeActionsByLines(
         ~handle,
         ~lines,
         ~buffer,
         ~context,
         ~toMsg,
         client,
       )
     })
  |> Isolinear.Sub.batch;
};
