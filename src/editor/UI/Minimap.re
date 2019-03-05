/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

/* open Reglfw.Glfw; */
open Revery.Draw
open Revery.UI;

open Oni_Core;

open Types;

let lineStyle = Style.[position(`Absolute), top(0)];

let tokensToElement = (transform, yOffset, tokens: list(Tokenizer.t)) => {
  let f = (token: Tokenizer.t) => {
    let tokenWidth =
      Index.toZeroBasedInt(token.endPosition)
      - Index.toZeroBasedInt(token.startPosition);
    /* let style = */
    /*   Style.[ */
    /*     position(`Absolute), */
    /*     layoutMode(`Minimal), */
    /*     top(0), */
    /*     left( */
    /*       Constants.default.minimapCharacterWidth */
    /*       * Index.toZeroBasedInt(token.startPosition), */
    /*     ), */
    /*     height(Constants.default.minimapCharacterHeight), */
    /*     width(tokenWidth * Constants.default.minimapCharacterWidth), */
    /*     backgroundColor(token.color), */
    /*   ]; */

    /* <View style />; */

    let x = float_of_int(Constants.default.minimapCharacterWidth * Index.toZeroBasedInt(token.startPosition));
    let height = float_of_int(Constants.default.minimapCharacterHeight);
    let width = float_of_int(tokenWidth * Constants.default.minimapCharacterWidth);

    Shapes.drawRect(~transform, ~y=float_of_int(yOffset), ~x=x, ~color=token.color, ~width, ~height, ());
  };

  List.iter(f, tokens);

  /* <View style=lineStyle> ...tokens </View>; */
};

let renderLine = (_theme, transform, yOffset, tokens) => {
  tokensToElement(transform, yOffset, tokens);
};

let component = React.component("Minimap");

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let createElement =
    (
      ~state: State.t,
      ~width: int,
      ~height: int,
      ~count,
      ~getTokensForLine: int => list(Tokenizer.t),
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let rowHeight =
      Constants.default.minimapCharacterHeight
      + Constants.default.minimapLineSpacing;

    let scrollY = state.editor.minimapScrollY;

    /* let render = i => { */
    /*   let tokens = getTokensForLine(i); */
    /*   renderLine(state.theme, tokens); */
    /* }; */

    /* ignore((width, height, count, rowHeight, render)); */
    ignore(width);

    (
      hooks,
      <View style=absoluteStyle>
        <OpenGL style=absoluteStyle render={(transform, _) => {
            FlatList.render(
                ~scrollY,
                ~rowHeight,
                ~height,
                ~count,
                ~render={(item, offset) => {
                   let tokens = getTokensForLine(item);
                   renderLine(state.theme, transform, offset, tokens) 
                }}, ());

        }} />
      </View>,
    );
  });
