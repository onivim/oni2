/*
 * HoverView.re
 *
 */

open Revery.UI;

module Zed_utf8 = Oni_Core.ZedBundled;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

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

  let text = (~colors: Colors.t, ~editorFont: Service_Font.font) => [
    //width(width_),
    //height(height_),
    //textWrap(TextWrapping.NoWrap),
    textOverflow(`Ellipsis),
    fontFamily(editorFont.fontFile),
    fontSize(editorFont.fontSize),
    color(colors.editorForeground),
    backgroundColor(colors.hoverWidgetBackground),
  ];

  let outerPosition = (~x, ~y) => [
    position(`Absolute),
    top(y - 4),
    left(x + 4),
  ];

  let innerPosition = (~width, ~height, ~colors: Colors.t) => [
    position(`Absolute),
    bottom(0),
    left(0),
    Style.width(width),
    Style.height(height),
    flexDirection(`Column),
    alignItems(`Center),
    justifyContent(`Center),
    border(~color=colors.hoverWidgetBorder, ~width=1),
    backgroundColor(colors.hoverWidgetBackground),
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
                ~colors,
                ~editorFont,
                (),
              ) => {
  let%hook (opacity, _, _) =
    Animation.animate(Revery.Time.ms(250))
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
        int_of_float(Service_Font.measure(~text, editorFont) +. 0.5);
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
      let fontHeight =
        int_of_float(Service_Font.getHeight(editorFont) +. (-1.5));
      let elementHeight = fontHeight + Constants.innerPadding;
      elementHeight * List.length(diagnostics) + Constants.padding * 2;
    };

    let elements =
      diagnostics
      |> List.map(({message, _}: Diagnostic.t) =>
           <Text style={Styles.text(~colors, ~editorFont)} text=message />
         )
      |> List.rev
      |> React.listToElement;

    <View style={Styles.outerPosition(~x, ~y)}>
      <Opacity opacity>
        <View style={Styles.innerPosition(~width, ~height, ~colors)}>
          elements
        </View>
      </Opacity>
    </View>;
  };
};

let make =
    (
      ~x,
      ~y,
      ~delay,
      ~isEnabled,
      ~colors,
      ~editorFont,
      ~diagnostics,
      ~editor: Editor.t,
      ~buffer,
      ~mode,
      (),
    ) =>
  if (isEnabled && mode != Vim.Types.Insert) {
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
              colors
              editorFont
            />
          )
       |> React.listToElement}
    </View>;
  } else {
    React.empty;
  };
