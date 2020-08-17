// [position] specifices the positioning preference for the section:
// below the target line, or above the target line
type position = [ | `Below];

module Section = {
  type t = {
    element: Revery.UI.element,
    position,
  };
};

open Revery.UI;
open Revery.UI.Components;

module Styles = {
  open Style;
  module Colors = Feature_Theme.Colors;

  let diagnostic = (~theme) => [
    textOverflow(`Ellipsis),
    color(Colors.Editor.foreground.from(theme)),
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
  ];

  let hr = (~theme) => [
    flexGrow(1),
    height(1),
    backgroundColor(Colors.EditorHoverWidget.border.from(theme)),
    marginTop(3),
    marginBottom(3),
  ];
};

let horizontalRule = (~theme, ()) =>
  <Row> <View style={Styles.hr(~theme)} /> </Row>;

let make =
    (
      ~x: int,
      ~topY: int,
      ~bottomY: int,
      ~availableWidth: int,
      ~availableHeight: int,
      ~sections: list(Section.t),
      ~theme: Oni_Core.ColorTheme.Colors.t,
      (),
    ) => {
  // TODO: Implement positioning logic
  ignore(topY);
  ignore(availableWidth);
  ignore(availableHeight);

  switch (sections) {
  | [] => React.empty
  | sections =>
    let element =
      sections
      |> List.map(({element, _}: Section.t) => element)
      |> Base.List.intersperse(~sep=<horizontalRule theme />)
      |> React.listToElement;

    <HoverView x y=bottomY theme> element </HoverView>;
  };
};
