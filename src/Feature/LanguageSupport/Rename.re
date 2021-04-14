open Oni_Core;
open Exthost;

[@deriving show]
type command =
  | RenameSymbol
  | Cancel
  | Commit;

[@deriving show]
type msg =
  | Noop
  | Command(command)
  | InputText(Component_InputText.msg)
  | RenameLocationAvailable({
      location: Exthost.RenameLocation.t,
      handle: int,
    })
  | RenameLocationUnavailable
  | RenameLocationError(string)
  // Candidate edits - preview of edits while user is typing
  | CandidateEditsFailed(string)
  | CandidateEditsAvailable(Exthost.WorkspaceEdit.t)
  | NoCandidateEditsAvailable
  // Actual rename edits
  | ApplyEditsFailed(string)
  | ApplyEditsAvailable(Exthost.WorkspaceEdit.t)
  | NoApplyEditsAvailable;

[@deriving show]
type provider = {
  handle: int,
  selector: DocumentSelector.t,
  supportsResolveInitialValues: bool,
};

[@deriving show]
type sessionState =
  | Inactive
  | Resolving
  | Resolved({
      handle: int, // provider handle for rename
      sessionId: int, //location: RenameLocation.t,
      inputText: [@opaque] Component_InputText.model,
      edits: option(Exthost.WorkspaceEdit.t),
    })
  | Applying({sessionId: int});

[@deriving show]
type model = {
  nextSessionId: int,
  sessionState,
  providers: list(provider),
};

let initial = {nextSessionId: 0, sessionState: Inactive, providers: []};

let register = (~handle, ~selector, ~supportsResolveInitialValues, model) => {
  {
    ...model,
    providers: [
      {handle, selector, supportsResolveInitialValues},
      ...model.providers,
    ],
  };
};

let unregister = (~handle, model) => {
  ...model,
  providers:
    model.providers |> List.filter(provider => provider.handle != handle),
};

let isFocused = ({sessionState, _}) => {
  switch (sessionState) {
  | Inactive => false
  | Resolving => false
  | Resolved(_) => true
  | Applying(_) => true
  };
};

let update = (~client, ~maybeBuffer, ~cursorLocation, msg, model) => {
  switch (msg) {
  | Noop => (model, Outmsg.Nothing)
  | InputText(inputMsg) =>
    switch (model.sessionState) {
    | Resolved({inputText, _} as resolved) =>
      let (inputText', _) = Component_InputText.update(inputMsg, inputText);
      (
        {
          ...model,
          sessionState: Resolved({...resolved, inputText: inputText'}),
        },
        Outmsg.Nothing,
      );
    | _ => (model, Outmsg.Nothing)
    }

  | RenameLocationAvailable({handle, location}) =>
    switch (model.sessionState) {
    | Resolving =>
      let sessionId = model.nextSessionId;
      let inputText =
        Component_InputText.create(~placeholder="")
        |> Component_InputText.set(~text=location.text)
        |> Component_InputText.selectAll;
      (
        {
          ...model,
          nextSessionId: sessionId + 1,
          sessionState: Resolved({handle, sessionId, inputText, edits: None}),
        },
        Outmsg.Nothing,
      );
    | _ => (model, Outmsg.Nothing)
    }

  | RenameLocationUnavailable => (model, Outmsg.Nothing)

  | RenameLocationError(msg) => (model, Outmsg.NotifyFailure(msg))

  | NoCandidateEditsAvailable => (model, Outmsg.Nothing)

  | CandidateEditsAvailable(edits) =>
    let sessionState' =
      switch (model.sessionState) {
      | Resolved(session) => Resolved({...session, edits: Some(edits)})
      | other => other
      };
    ({...model, sessionState: sessionState'}, Outmsg.Nothing);

  | CandidateEditsFailed(_err) => (model, Outmsg.Nothing)

  | ApplyEditsFailed(_)
  | NoApplyEditsAvailable => (
      {...model, sessionState: Inactive},
      Outmsg.Nothing,
    )

  | Command(Commit) =>
    switch (model.sessionState) {
    | Inactive
    | Resolving
    | Applying(_) => (model, Outmsg.Nothing)
    | Resolved({handle, inputText, sessionId, _}) =>
      let eff =
        maybeBuffer
        |> Option.map(buffer => {
             Service_Exthost.Effects.LanguageFeatures.provideRenameEdits(
               ~handle,
               ~position=cursorLocation,
               ~newName=Component_InputText.value(inputText),
               ~uri=Buffer.getUri(buffer),
               client,
               fun
               | Ok(Some(edit)) => ApplyEditsAvailable(edit)
               | Ok(None) => NoApplyEditsAvailable
               | Error(msg) => ApplyEditsFailed(msg),
             )
           })
        |> Option.value(~default=Isolinear.Effect.none);

      (
        {...model, sessionState: Applying({sessionId: sessionId})},
        Outmsg.Effect(eff),
      );
    }

  | ApplyEditsAvailable(edit) =>
    let outmsg =
      switch (model.sessionState) {
      | Applying(_) => Outmsg.ApplyWorkspaceEdit(edit)
      | Inactive
      | Resolving
      | Resolved(_) => Outmsg.Nothing
      };

    ({...model, sessionState: Inactive}, outmsg);

  | Command(Cancel) => ({...model, sessionState: Inactive}, Outmsg.Nothing)
  | Command(RenameSymbol) =>
    switch (maybeBuffer) {
    | None => (model, Outmsg.Nothing)
    | Some(buffer) =>
      let toMsg = (handle, maybeLocationResult) => {
        switch (maybeLocationResult) {
        | Ok(Some(location)) => RenameLocationAvailable({location, handle})
        | Ok(None) => RenameLocationUnavailable
        | Error(msg) => RenameLocationError(msg)
        };
      };
      let eff =
        model.providers
        |> List.filter(provider =>
             Exthost.DocumentSelector.matchesBuffer(
               ~buffer,
               provider.selector,
             )
           )
        |> List.map(provider => {
             Service_Exthost.Effects.LanguageFeatures.resolveRenameLocation(
               ~handle=provider.handle,
               ~uri=Oni_Core.Buffer.getUri(buffer),
               ~position=cursorLocation,
               client,
               toMsg(provider.handle),
             )
           })
        |> Isolinear.Effect.batch;

      ({...model, sessionState: Resolving}, Outmsg.Effect(eff));
    }
  };
};

let keyPressed = (key, model) => {
  let sessionState' =
    switch (model.sessionState) {
    | Resolved(state) =>
      Resolved({
        ...state,
        inputText: Component_InputText.handleInput(~key, state.inputText),
      })
    | state => state
    };
  {...model, sessionState: sessionState'};
};

let sub = (~activeBuffer, ~activePosition, ~client, model) =>
  switch (model.sessionState) {
  | Resolving
  | Inactive
  | Applying(_) => Isolinear.Sub.none

  | Resolved({handle, inputText, _}) =>
    let toMsg = (
      fun
      | Ok(Some(edit)) => CandidateEditsAvailable(edit)
      | Ok(None) => NoCandidateEditsAvailable
      | Error(msg) => CandidateEditsFailed(msg)
    );

    Service_Exthost.Sub.renameEdits(
      ~handle,
      ~buffer=activeBuffer,
      ~position=activePosition,
      ~newName=Component_InputText.value(inputText),
      ~toMsg,
      client,
    );
  };

module Commands = {
  open Feature_Commands.Schema;

  let rename =
    define(
      ~category="Language Support",
      ~title="Rename Symbol",
      "editor.action.rename",
      Command(RenameSymbol),
    );

  let cancel = define("editor.action.rename.cancel", Command(Cancel));

  let commit = define("editor.action.rename.commit", Command(Commit));
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  // TODO:
  let renameInputVisible = bool("renameInputVisible", isFocused);
};

module Keybindings = {
  open Feature_Input.Schema;

  let condition = "normalMode" |> WhenExpr.parse;

  let rename = bind(~key="<F2>", ~command=Commands.rename.id, ~condition);

  let activeCondition = "renameInputVisible" |> WhenExpr.parse;

  let cancel =
    bind(
      ~key="<ESC>",
      ~command=Commands.cancel.id,
      ~condition=activeCondition,
    );
  let commit =
    bind(
      ~key="<CR>",
      ~command=Commands.commit.id,
      ~condition=activeCondition,
    );
};

module Contributions = {
  let commands = Commands.[rename, cancel, commit];

  let contextKeys = [ContextKeys.renameInputVisible];

  let keybindings = Keybindings.[rename, cancel, commit];
};

module View = {
  open Revery;
  open Revery.UI;
  module Styles = {
    open Style;
    let color = Color.rgba(0., 0., 0., 0.75);
    let boxShadow = (x, y) => [
      position(`Absolute),
      top(y),
      left(x),
      boxShadow(
        ~xOffset=4.,
        ~yOffset=4.,
        ~blurRadius=12.,
        ~spreadRadius=0.,
        ~color,
      ),
    ];
  };
  module Colors = Feature_Theme.Colors;
  let make =
      (
        ~x,
        ~y,
        ~theme: ColorTheme.Colors.t,
        ~model: model,
        ~font: UiFont.t,
        ~dispatch: msg => unit,
        (),
      ) => {
    switch (model.sessionState) {
    | Resolved({inputText, _}) =>
      <View style={Styles.boxShadow(x, y)}>
        <View
          style=Style.[
            width(400),
            backgroundColor(Colors.Menu.background.from(theme)),
            color(Colors.Menu.foreground.from(theme)),
          ]>
          <View style=Style.[padding(5)]>
            <Component_InputText.View
              model=inputText
              theme
              isFocused=true
              fontFamily={font.family}
              fontSize=14.
              dispatch={msg => dispatch(InputText(msg))}
            />
          </View>
        </View>
      </View>
    | _ => React.empty
    };
  };
};
