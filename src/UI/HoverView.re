/*
 * HoverView.re
 *
 */

open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Model;
open Oni_Core_Utility;

module Zed_utf8 = Oni_Core_Kernel.ZedBundled;

module Constants = {
  let padding = 8;
  let innerPadding = 1;
};

module Styles = {
  open Style;

  let container = [
    position(`Absolute),
    top(0),
    left(0),
    bottom(0),
    right(0),
  ];

  let text = (~theme: Theme.t, ~editorFont: EditorFont.t) => [
    //width(width_),
    //height(height_),
    //textWrap(TextWrapping.NoWrap),
    textOverflow(`Ellipsis),
    fontFamily(editorFont.fontFile),
    fontSize(editorFont.fontSize),
    color(theme.editorForeground),
    backgroundColor(theme.editorHoverWidgetBackground),
  ];

  let outerPosition = (~x, ~y) => [
    position(`Absolute),
    top(y - 4),
    left(x + 4),
  ];

  let innerPosition = (~width, ~height, ~theme: Theme.t) => [
    position(`Absolute),
    bottom(0),
    left(0),
    Style.width(width),
    Style.height(height),
    flexDirection(`Column),
    alignItems(`Center),
    justifyContent(`Center),
    border(~color=theme.editorHoverWidgetBorder, ~width=1),
    backgroundColor(theme.editorHoverWidgetBackground),
  ];
};

let%component hoverItem =
              (
                ~x,
                ~y,
                ~diagnostics,
                ~buffer,
                ~location,
                ~delay,
                ~theme,
                ~editorFont,
                (),
              ) => {
  let%hook (opacity, _, _) =
    Animation.animate(Time.ms(250))
    |> Animation.delay(delay)
    |> Animation.tween(0., 1.)
    |> Hooks.animation;

  let diagnostics =
    Diagnostics.getDiagnosticsAtPosition(diagnostics, buffer, location);

  if (diagnostics == []) {
    React.empty;
  } else {
    let width = {
      let measure = text =>
        int_of_float(EditorFont.measure(~text, editorFont) +. 0.5);
      let maxElementWidth =
        List.fold_left(
          (maxWidth, {message, _}: Diagnostic.t) =>
            max(maxWidth, measure(message) + Constants.padding),
          0,
          diagnostics,
        );
      maxElementWidth + Constants.padding * 2;
    };

    let height = {
      let fontHeight = int_of_float(EditorFont.getHeight(editorFont) +. 0.5);
      let elementHeight = fontHeight + Constants.innerPadding;
      elementHeight * List.length(diagnostics) + Constants.padding * 2;
    };

    let elements =
      diagnostics
      |> List.map(({message, _}: Diagnostic.t) =>
           <Text style={Styles.text(~theme, ~editorFont)} text=message />
         )
      |> List.rev
      |> React.listToElement;

    <View style={Styles.outerPosition(~x, ~y)}>
      <Opacity opacity>
        <View style={Styles.innerPosition(~width, ~height, ~theme)}>
          elements
        </View>
      </Opacity>
    </View>;
  };
};

let make = (~x, ~y, ~state: State.t, ()) => {
  let delay =
    Configuration.getValue(c => c.editorHoverDelay, state.configuration)
    |> Time.ms;
  let hoverEnabled =
    Configuration.getValue(c => c.editorHoverEnabled, state.configuration);

  let State.{theme, editorFont, diagnostics, _} = state;
  let maybeEditor =
    state |> Selectors.getActiveEditorGroup |> Selectors.getActiveEditor;

  switch (maybeEditor) {
  | Some(editor) when hoverEnabled && state.mode != Vim.Types.Insert =>
    switch (Buffers.getBuffer(editor.bufferId, state.buffers)) {
    | Some(buffer) =>
      <View style=Styles.container>
        {editor.cursors
         |> List.map(cursor =>
              <hoverItem
                x
                y
                diagnostics
                buffer
                location=cursor
                delay
                theme
                editorFont
              />
            )
         |> React.listToElement}
      </View>

    | None => React.empty
    }

  | _ => React.empty
  };
};
