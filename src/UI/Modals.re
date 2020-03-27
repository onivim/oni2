open Revery;
open Revery.UI;

open Oni_Core;
open Utility;
open Oni_Model;
open Actions;
open Oni_Components;
open Modal;

let unsavedBuffersWarning =
  UnsavedBuffersWarning({
    input: "",
    actions: [
      {
        label: "Discard Changes",
        msg: WindowCloseDiscardConfirmed,
        shortcut: Sequence(":qa!"),
      },
      {label: "Cancel", msg: WindowCloseCanceled, shortcut: Key("<ESC>")},
      {
        label: "Save All",
        msg: WindowCloseSaveAllConfirmed,
        shortcut: Sequence(":xa"),
      },
    ],
  });

let update = (state, msg) =>
  switch (state, msg) {
  | (UnsavedBuffersWarning(model), KeyPressed(key)) =>
    let (model, eff) = MessageBox.update(model, MessageBox.KeyPressed(key));
    (UnsavedBuffersWarning(model), eff);
  };

module Styles = {
  open Style;

  let overlay = [
    backgroundColor(Color.hex("#0004")),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    alignItems(`Center),
    justifyContent(`Center),
    overflow(`Hidden),
    flexDirection(`Column),
    pointerEvents(`Allow),
  ];

  let text = (~theme: Theme.t, ~font: UiFont.t) => [
    fontFamily(font.fontFile),
    color(theme.foreground),
    backgroundColor(theme.editorBackground),
    fontSize(14.),
    textWrap(TextWrapping.NoWrap),
  ];

  let files = [padding(10)];

  let file = (~theme: Theme.t, ~font: UiFont.t) => [
    fontFamily(font.fontFile),
    color(theme.foreground),
    backgroundColor(theme.editorBackground),
    fontSize(14.),
    textWrap(TextWrapping.NoWrap),
  ];
};

let unsavedBufferWarning =
    (~model, ~workingDirectory, ~buffers, ~theme, ~uiFont as font, ()) => {
  let modifiedFiles =
    buffers
    |> IntMap.to_seq
    |> Seq.map(snd)
    |> Seq.filter(Buffer.isModified)
    |> Seq.map(Buffer.getFilePath)
    |> List.of_seq
    |> OptionEx.values
    |> List.map(
         Path.toRelative(~base=Option.value(workingDirectory, ~default="")),
       );

  <MessageBox model onAction={GlobalContext.current().dispatch} theme font>
    <Text
      style={Styles.text(~theme, ~font)}
      text="You have unsaved changes in the following files:"
    />
    <View style=Styles.files>
      {modifiedFiles
       |> List.map(text => <Text style={Styles.file(~theme, ~font)} text />)
       |> React.listToElement}
    </View>
    <Text
      style={Styles.text(~theme, ~font)}
      text="Would you like to to save them before closing?"
    />
  </MessageBox>;
};

let make = (~state: State.t, ()) => {
  let State.{theme, uiFont, buffers, workspace, _} = state;
  let workingDirectory =
    Option.map(ws => ws.Workspace.workingDirectory, workspace);

  switch (state.modal) {
  | None => React.empty
  | Some(modal) =>
    <View style=Styles.overlay>
      {switch (modal) {
       | UnsavedBuffersWarning(model) =>
         <unsavedBufferWarning model workingDirectory buffers theme uiFont />
       }}
    </View>
  };
};
