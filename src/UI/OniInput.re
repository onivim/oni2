open Revery;
open Revery.UI;
open Revery.UI.Components;

module Option = Oni_Core.Utility.Option;

module Cursor = {
  type state = {
    time: Time.t,
    isOn: bool,
  };

  type action =
    | Reset
    | Tick(Time.t);

  let use = (~interval, ~isFocused) => {
    let%hook (state, dispatch) =
      Hooks.reducer(
        ~initialState={time: Time.zero, isOn: false}, (action, state) => {
        switch (action) {
        | Reset => {isOn: true, time: Time.zero}
        | Tick(increasedTime) =>
          let newTime = Time.(state.time + increasedTime);

          /* if newTime is above the interval a `Tick` has passed */
          newTime >= interval
            ? {isOn: !state.isOn, time: Time.zero}
            : {...state, time: newTime};
        }
      });

    let%hook () =
      Hooks.effect(
        OnMount,
        () => {
          let clear =
            Tick.interval(time => dispatch(Tick(time)), Time.ms(16));
          Some(clear);
        },
      );

    let cursorOpacity = isFocused && state.isOn ? 1.0 : 0.0;

    (cursorOpacity, () => dispatch(Reset));
  };
};

type changeEvent = {
  value: string,
  character: string,
  keycode: Key.Keycode.t,
  altKey: bool,
  ctrlKey: bool,
  shiftKey: bool,
  superKey: bool,
};

let getStringParts = (index, str) => {
  switch (index) {
  | 0 => ("", str)
  | _ =>
    let strBeginning =
      try(Str.string_before(str, index)) {
      | _ => str
      };
    let strEnd =
      try(Str.string_after(str, index)) {
      | _ => ""
      };
    (strBeginning, strEnd);
  };
};

let getSafeStringBounds = (str, cursorPosition, change) => {
  let nextPosition = cursorPosition + change;
  let currentLength = String.length(str);
  nextPosition > currentLength
    ? currentLength : nextPosition < 0 ? 0 : nextPosition;
};

let removeCharacterBefore = (word, cursorPosition) => {
  let (startStr, endStr) = getStringParts(cursorPosition, word);
  let nextPosition = getSafeStringBounds(startStr, cursorPosition, -1);
  let newString = Str.string_before(startStr, nextPosition) ++ endStr;
  (newString, nextPosition);
};

let removeCharacterAfter = (word, cursorPosition) => {
  let (startStr, endStr) = getStringParts(cursorPosition, word);
  let newString =
    startStr
    ++ (
      switch (endStr) {
      | "" => ""
      | _ => Str.last_chars(endStr, String.length(endStr) - 1)
      }
    );
  (newString, cursorPosition);
};

let addCharacter = (word, char, index) => {
  let (startStr, endStr) = getStringParts(index, word);
  (startStr ++ char ++ endStr, String.length(startStr) + 1);
};

module Constants = {
  let cursorWidth = 2;
};

module Styles = {
  let defaultPlaceholderColor = Colors.grey;
  let defaultCursorColor = Colors.black;

  let default =
    Style.[
      color(Colors.black),
      paddingVertical(8),
      paddingHorizontal(12),
      border(~width=1, ~color=Colors.black),
      backgroundColor(Colors.transparentWhite),
    ];
};

let%component make =
              (
                ~style=Styles.default,
                ~placeholderColor=Styles.defaultPlaceholderColor,
                ~cursorColor=Styles.defaultCursorColor,
                ~placeholder="",
                ~prefix="",
                ~isFocused,
                ~value,
                ~cursorPosition,
                ~onClick,
                (),
              ) => {
  let%hook (textRef, setTextRef) = Hooks.ref(None);
  let%hook (scrollOffset, _setScrollOffset) = Hooks.state(ref(0));

  let displayValue = prefix ++ value;
  let showPlaceholder = displayValue == "";

  module Styles = {
    open Style;
    include Styles;

    let fontSize = Selector.select(style, FontSize, 18);
    let textColor = Selector.select(style, Color, Colors.black);
    let fontFamily = Selector.select(style, FontFamily, "Roboto-Regular.ttf");

    let _all =
      merge(
        ~source=[
          flexDirection(`Row),
          alignItems(`Center),
          justifyContent(`FlexStart),
          overflow(`Hidden),
          cursor(MouseCursors.text),
          ...default,
        ],
        ~target=style,
      );

    let box = extractViewStyles(_all);

    let marginContainer = [
      flexDirection(`Row),
      alignItems(`Center),
      justifyContent(`FlexStart),
      flexGrow(1),
    ];

    let cursor = offset => [
      position(`Absolute),
      marginTop(2),
      transform(Transform.[TranslateX(float(offset))]),
    ];

    let textContainer = [flexGrow(1), overflow(`Hidden)];

    let text = [
      color(showPlaceholder ? placeholderColor : textColor),
      Style.fontFamily(fontFamily),
      Style.fontSize(fontSize),
      alignItems(`Center),
      justifyContent(`FlexStart),
      textWrap(TextWrapping.NoWrap),
      transform(Transform.[TranslateX(float(- scrollOffset^))]),
    ];
  };

  let measureTextWidth = text => {
    let dimensions =
      Revery_Draw.Text.measure(
        ~fontFamily=Styles.fontFamily,
        ~fontSize=Styles.fontSize,
        text,
      );

    dimensions.width;
  };

  let%hook (cursorOpacity, resetCursor) =
    Cursor.use(~interval=Time.ms(500), ~isFocused);

  let%hook () =
    Hooks.effect(
      If((!=), (value, cursorPosition, isFocused)),
      () => {
        resetCursor();
        None;
      },
    );

  let () = {
    let cursorOffset =
      measureTextWidth(String.sub(displayValue, 0, cursorPosition));

    switch (Option.bind(r => r#getParent(), textRef)) {
    | Some(containerNode) =>
      let container: Dimensions.t = containerNode#measurements();

      if (cursorOffset < scrollOffset^) {
        // out of view to the left, so align with left edge
        scrollOffset := cursorOffset;
      } else if (cursorOffset - scrollOffset^ > container.width) {
        // out of view to the right, so align with right edge
        scrollOffset := cursorOffset - container.width;
      };

    | None => ()
    };
  };

  let handleClick = (event: NodeEvents.mouseButtonEventParams) => {
    let rec offsetLeft = node => {
      let Dimensions.{left, _} = node#measurements();
      switch (node#getParent()) {
      | Some(parent) => left + offsetLeft(parent)
      | None => left
      };
    };

    let indexNearestOffset = offset => {
      let rec loop = (i, last) =>
        if (i > String.length(value)) {
          i - 1;
        } else {
          let width = measureTextWidth(String.sub(value, 0, i));

          if (width > offset) {
            let isCurrentNearest = width - offset < offset - last;
            isCurrentNearest ? i : i - 1;
          } else {
            loop(i + 1, width);
          };
        };

      loop(1, 0);
    };

    switch (textRef) {
    | Some(node) =>
      let offset =
        int_of_float(event.mouseX) - offsetLeft(node) + scrollOffset^;
      let cursorPosition = indexNearestOffset(offset);
      resetCursor();
      onClick(cursorPosition);

    | None => ()
    };
  };

  let cursor = () => {
    let (startStr, _) =
      getStringParts(cursorPosition + String.length(prefix), value);
    let textWidth = measureTextWidth(startStr);

    let offset = textWidth - scrollOffset^;

    <View style={Styles.cursor(offset)}>
      <Opacity opacity=cursorOpacity>
        <Container
          width=Constants.cursorWidth
          height=Styles.fontSize
          color=cursorColor
        />
      </Opacity>
    </View>;
  };

  let text = () =>
    <Text
      ref={node => setTextRef(Some(node))}
      text={showPlaceholder ? placeholder : displayValue}
      style=Styles.text
    />;

  <Clickable onAnyClick=handleClick>
    <View style=Styles.box>
      <View style=Styles.marginContainer>
        <cursor />
        <View style=Styles.textContainer> <text /> </View>
      </View>
    </View>
  </Clickable>;
};
