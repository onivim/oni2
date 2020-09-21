open EditorCoreTypes;
open Revery;
open Revery.UI;
open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.UI.LocationListItem"));

module Colors = Feature_Theme.Colors;

type t = {
  file: string,
  location: CharacterPosition.t,
  text: string,
  highlight: option((Index.t, Index.t)),
};

module Styles = {
  open Style;

  let clickable = [cursor(Revery.MouseCursors.pointer)];

  let result = (~theme, ~isHovered) => [
    flexDirection(`Row),
    overflow(`Hidden),
    paddingVertical(4),
    paddingHorizontal(8),
    backgroundColor(
      isHovered
        ? Colors.Menu.selectionBackground.from(theme)
        : Revery.Colors.transparentWhite,
    ),
  ];

  let locationText = (~theme) => [
    color(Colors.EditorLineNumber.activeForeground.from(theme)),
    textWrap(TextWrapping.NoWrap),
  ];

  let snippet = (~theme, ~isHighlighted) => [
    color(
      isHighlighted
        ? Colors.Oni.normalModeBackground.from(theme)
        : Colors.foreground.from(theme),
    ),
    textWrap(TextWrapping.NoWrap),
  ];
};

module View = {
  let make =
      (
        ~theme,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        ~width,
        ~isHovered,
        ~item,
        ~workingDirectory,
        (),
      ) => {
    //  let workingDirectory = Rench.Environment.getWorkingDirectory(); // TODO: This should be workspace-relative

    let locationText =
      Printf.sprintf(
        "%s:%n - ",
        Path.toRelative(~base=workingDirectory, item.file),
        EditorCoreTypes.LineNumber.toOneBased(item.location.line),
      );

    let locationWidth = {
      Revery.Draw.Text.dimensions(
        ~smoothing=Revery.Font.Smoothing.default,
        ~fontSize=uiFont.size,
        ~fontFamily=uiFont.family,
        ~fontWeight=Normal,
        locationText,
      ).
        width;
    };

    let location = () =>
      <Text
        style={Styles.locationText(~theme)}
        fontFamily={uiFont.family}
        fontSize={uiFont.size}
        text=locationText
      />;

    let content = () => {
      let unstyled = (~text, ()) =>
        <Text
          style={Styles.snippet(~theme, ~isHighlighted=false)}
          fontFamily={editorFont.fontFamily}
          fontSize={editorFont.fontSize}
          text
        />;

      let highlighted = (~text, ()) =>
        <Text
          style={Styles.snippet(~theme, ~isHighlighted=true)}
          fontFamily={editorFont.fontFamily}
          fontSize={editorFont.fontSize}
          text
        />;

      switch (item.highlight) {
      | Some((indexStart, indexEnd)) =>
        let availableWidth = float(width) -. locationWidth;
        let maxLength = int_of_float(availableWidth /. editorFont.spaceWidth);
        let charStart = Index.toZeroBased(indexStart);
        let charEnd = Index.toZeroBased(indexEnd);

        try({
          let (text, charStart, charEnd) =
            StringEx.extractSnippet(
              ~maxLength,
              ~charStart,
              ~charEnd,
              item.text,
            );
          let before = String.sub(text, 0, charStart);
          let matchedText = String.sub(text, charStart, charEnd - charStart);
          let after =
            String.sub(text, charEnd, String.length(text) - charEnd);

          <View style=Style.[flexDirection(`Row)]>
            <unstyled text=before />
            <highlighted text=matchedText />
            <unstyled text={StringEx.trimRight(after)} />
          </View>;
        }) {
        | Invalid_argument(message) =>
          // TODO: This shouldn't happen, but you never know. Consider a sane implementation of `String.sub` instead, to avoid this
          Log.errorf(m =>
            m("\"%s\" - (%n, %n)\n%!", message, charStart, charEnd)
          );
          <unstyled text={item.text} />;
        };
      | None => <unstyled text={item.text} />
      };
    };

    <View style=Styles.clickable>
      <View style={Styles.result(~theme, ~isHovered)}>
        <location />
        <content />
      </View>
    </View>;
  };
};
