open Oni_Components;

// MODEL

[@deriving show({with_path: false})]
type msg =
  | WindowCloseDiscardConfirmed
  | WindowCloseSaveAllConfirmed
  | WindowCloseCanceled
  | WriteFailureDiscardConfirmed
  | WriteFailureOverwriteConfirmed
  | WriteFailureCanceled
  | KeyPressed(string);

type model =
  | WriteFailure(MessageBox.model(msg))
  | UnsavedBuffersWarning(MessageBox.model(msg));

// UPDATE

type outmsg =
  | ChoiceConfirmed(Isolinear.Effect.t(msg))
  | Effect(Isolinear.Effect.t(msg));

let update = (model, msg) =>
  switch (msg) {
  | KeyPressed(key) =>
    switch (model) {
    | UnsavedBuffersWarning(model) =>
      let (model, eff) =
        MessageBox.update(model, MessageBox.KeyPressed(key));
      (UnsavedBuffersWarning(model), Effect(eff));

    | WriteFailure(model) =>
      let (model, eff) =
        MessageBox.update(model, MessageBox.KeyPressed(key));
      (WriteFailure(model), Effect(eff));
    }

  | WindowCloseDiscardConfirmed => (
      model,
      ChoiceConfirmed(Service_Vim.quitAll()),
    )

  | WindowCloseSaveAllConfirmed => (
      model,
      ChoiceConfirmed(Service_Vim.saveAllAndQuit()),
    )

  | WindowCloseCanceled => (model, ChoiceConfirmed(Isolinear.Effect.none))

  | WriteFailureDiscardConfirmed => (
      model,
      ChoiceConfirmed(Service_Vim.forceReload()),
    )

  | WriteFailureOverwriteConfirmed => (
      model,
      ChoiceConfirmed(Service_Vim.forceOverwrite()),
    )

  | WriteFailureCanceled => (model, ChoiceConfirmed(Isolinear.Effect.none))
  };

// VIEW

module View = {
  open Revery;
  open Revery.UI;

  open Oni_Core;
  open Utility;

  module Colors = Feature_Theme.Colors;

  module Styles = {
    open Style;

    let backdrop = (~theme) => [
      backgroundColor(Colors.Oni.Modal.backdrop.from(theme)),
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

    let text = (~theme) => [
      color(Colors.Oni.Modal.foreground.from(theme)),
      textWrap(TextWrapping.NoWrap),
    ];

    let files = [padding(10)];

    let file = (~theme) => [
      color(
        Colors.Oni.Modal.foreground.from(theme) |> Color.multiplyAlpha(0.75),
      ),
      textWrap(TextWrapping.NoWrap),
    ];
  };

  let unsavedBuffersWarning =
      (
        ~model,
        ~workingDirectory,
        ~buffers,
        ~theme,
        ~font: UiFont.t,
        ~dispatch,
        (),
      ) => {
    let modifiedFiles =
      buffers
      |> IntMap.to_seq
      |> Seq.map(snd)
      |> Seq.filter(Buffer.isModified)
      |> Seq.map(Buffer.getFilePath)
      |> List.of_seq
      |> OptionEx.values
      |> List.map(Path.toRelative(~base=workingDirectory));

    <MessageBox model onAction=dispatch theme font>
      <Text
        style={Styles.text(~theme)}
        fontFamily={font.normal}
        fontSize={font.size}
        text="You have unsaved changes in the following files:"
      />
      <View style=Styles.files>
        {modifiedFiles
         |> List.map(text =>
              <Text
                style={Styles.file(~theme)}
                fontFamily={font.semiBold}
                fontSize={font.size}
                text
              />
            )
         |> React.listToElement}
      </View>
      <Text
        style={Styles.text(~theme)}
        fontFamily={font.normal}
        fontSize={font.size}
        text="Would you like to to save them before closing?"
      />
    </MessageBox>;
  };

  let writeFailure = (~model, ~theme, ~font: UiFont.t, ~dispatch, ()) => {
    <MessageBox model onAction=dispatch theme font>
      <Text
        style={Styles.text(~theme)}
        fontFamily={font.normal}
        fontSize={font.size}
        text="Unable to save gracefully because the file has changed on disk."
      />
      <Text
        style={Styles.text(~theme)}
        fontFamily={font.normal}
        fontSize={font.size}
        text="Would you like to force overwrite or discard the unsaved changes?"
      />
    </MessageBox>;
  };

  let make =
      (~model, ~buffers, ~workingDirectory, ~theme, ~font, ~dispatch, ()) => {
    <View style={Styles.backdrop(~theme)}>
      {switch (model) {
       | UnsavedBuffersWarning(model) =>
         <unsavedBuffersWarning
           model
           workingDirectory
           buffers
           theme
           font
           dispatch
         />

       | WriteFailure(model) => <writeFailure model theme font dispatch />
       }}
    </View>;
  };
};

// INITIALIZERS

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

let writeFailure =
  WriteFailure({
    input: "",
    actions: [
      {
        label: "Discard Changes",
        msg: WriteFailureDiscardConfirmed,
        shortcut: Sequence(":e!"),
      },
      {label: "Cancel", msg: WriteFailureCanceled, shortcut: Key("<ESC>")},
      {
        label: "Overwrite",
        msg: WriteFailureOverwriteConfirmed,
        shortcut: Sequence(":w!"),
      },
    ],
  });
