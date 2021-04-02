open Oni_Core;
open Exthost;

type provider = {
  handle: int,
  selector: DocumentSelector.t,
  metadata: CodeAction.ProviderMetadata.t,
  displayName: string,
  supportsResolve: bool,
};

module CodeAction = {
  type t = {
    handle: int,
    action: Exthost.CodeAction.t,
  };

  let ofExthost = (~handle, action) => {handle, action};
};

module QuickFixes = {
  type t = {
    bufferId: int,
    position: EditorCoreTypes.CharacterPosition.t,
    fixes: list(CodeAction.t),
  };

  let addQuickFixes = (~handle, ~bufferId, ~position, ~newActions, quickFixes) => {
    let actions = newActions |> List.map(CodeAction.ofExthost(~handle));
    // If at the same location, just append!
    let fixes =
      if (bufferId == quickFixes.bufferId && position == quickFixes.position) {
        quickFixes.fixes @ actions;
      } else {
        // If not, replace
        actions;
      };

    {bufferId, position, fixes};
  };
  let initial = {
    bufferId: (-1),
    position: EditorCoreTypes.CharacterPosition.zero,
    fixes: [],
  };
};

type model = {
  providers: list(provider),
  quickFixes: QuickFixes.t,
};

[@deriving show]
type msg =
  | QuickFixesAvailable({
      handle: int,
      actions: list(Exthost.CodeAction.t),
    })
  | QuickFixesNotAvailable({handle: int})
  | QuickFixesError({
      handle: int,
      msg: string,
    });

let initial = {providers: [], quickFixes: QuickFixes.initial};

let register =
    (~handle, ~selector, ~metadata, ~displayName, ~supportsResolve, model) => {
  {
    ...model,
    providers: [
      {handle, selector, metadata, displayName, supportsResolve},
      ...model.providers,
    ],
  };
};

let unregister = (~handle, model) => {
  ...model,
  providers:
    model.providers |> List.filter(provider => {provider.handle != handle}),
};

let update = (~buffer, ~cursorLocation, msg, model) => {
  let bufferId = Buffer.getId(buffer);
  switch (msg) {
  | QuickFixesAvailable({handle, actions}) => (
      {
        ...model,
        quickFixes:
          QuickFixes.addQuickFixes(
            ~handle,
            ~bufferId,
            ~position=cursorLocation,
            ~newActions=actions,
            model.quickFixes,
          ),
      },
      Outmsg.Nothing,
    )
  | QuickFixesNotAvailable(_) => (model, Outmsg.Nothing)
  | QuickFixesError(_) => (model, Outmsg.Nothing)
  };
};

let sub =
    (
      ~buffer,
      ~activePosition,
      ~topVisibleBufferLine,
      ~bottomVisibleBufferLine,
      ~client,
      codeActions,
    ) => {
  // TODO:
  let context =
    Exthost.CodeAction.(Context.{only: None, trigger: TriggerType.Auto});

  let toMsg = handle =>
    fun
    | Ok(Some(actionsList: Exthost.CodeAction.List.t)) =>
      QuickFixesAvailable({handle, actions: actionsList.actions})
    | Ok(None) => QuickFixesNotAvailable({handle: handle})
    | Error(msg) => QuickFixesError({handle, msg});

  let range =
    EditorCoreTypes.CharacterRange.{
      start: activePosition,
      stop: activePosition,
    };

  codeActions.providers
  |> List.filter(({selector, _}) => {
       Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
     })
  |> List.map(({handle, _}) => {
       Service_Exthost.Sub.codeActionsByRange(
         ~handle,
         ~range,
         ~buffer,
         ~context,
         ~toMsg=toMsg(handle),
         client,
       )
     })
  |> Isolinear.Sub.batch;
};
