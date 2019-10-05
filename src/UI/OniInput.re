open Revery;
open Revery.UI;
open Revery.UI.Components;

type textUpdate = {
  newString: string,
  newCursorPosition: int,
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
  {newString, newCursorPosition: nextPosition};
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
  {newString, newCursorPosition: cursorPosition};
};

let deleteWord = (str, cursorPosition) => {
  let positionToDeleteTo =
    switch (String.rindex_from_opt(str, cursorPosition - 1, ' ')) {
    | None => 0
    | Some(v) => v
    };

  // Get the 'before' position
  let beforeStr =
    if (positionToDeleteTo > 0) {
      String.sub(str, 0, positionToDeleteTo);
    } else {
      "";
    };

  let afterStr =
    if (cursorPosition <= String.length(str) - 1) {
      String.sub(str, cursorPosition, String.length(str) - cursorPosition);
    } else {
      "";
    };
  {newString: beforeStr ++ afterStr, newCursorPosition: positionToDeleteTo};
};

let addCharacter = (word, char, index) => {
  let (startStr, endStr) = getStringParts(index, word);
  {
    newString: startStr ++ char ++ endStr,
    newCursorPosition: String.length(startStr) + 1,
  };
};

let defaultHeight = 40;
let defaultWidth = 200;
let inputTextMargin = 10;

let defaultStyles =
  Style.[
    color(Colors.black),
    width(defaultWidth),
    height(defaultHeight),
    border(
      /*
         The default border width should be 5% of the full input height
       */
      ~width=float_of_int(defaultHeight) *. 0.05 |> int_of_float,
      ~color=Colors.black,
    ),
    backgroundColor(Colors.transparentWhite),
  ];

let component = React.component("Input");
let make =
    (
      ~style,
      ~prefix="",
      ~autofocus,
      ~placeholder,
      ~cursorColor,
      ~placeholderColor,
      ~onChange,
      ~onKeyDown,
      ~fontSize,
      ~cursorPosition,
      ~text,
      (),
    ) =>
  component(hooks => {
    let valueToDisplay = prefix ++ text;
    let showPlaceholder = valueToDisplay == "";

    let handleKeyPress = (event: NodeEvents.keyPressEventParams) => {
      let {newString, newCursorPosition} =
        addCharacter(text, event.character, cursorPosition);
      onChange(newString, newCursorPosition);
    };

    let handleKeyDown = (event: NodeEvents.keyEventParams) => {
      switch (event.key) {
      | Key.KEY_LEFT =>
        onKeyDown(event);
        onChange(text, getSafeStringBounds(text, cursorPosition, -1));

      | Key.KEY_RIGHT =>
        onKeyDown(event);
        onChange(text, getSafeStringBounds(text, cursorPosition, 1));

      | Key.KEY_U when event.ctrlKey =>
        onChange("", 0);

      | Key.KEY_W when event.ctrlKey =>
        let {newString, newCursorPosition} =
          deleteWord(text, cursorPosition);
        onChange(newString, newCursorPosition);

      | Key.KEY_DELETE =>
        let {newString, newCursorPosition} =
          removeCharacterAfter(text, cursorPosition);
        onChange(newString, newCursorPosition);

      | Key.KEY_H when event.ctrlKey =>
        let {newString, newCursorPosition} =
          removeCharacterBefore(text, cursorPosition);
        onChange(newString, newCursorPosition);

      | Key.KEY_BACKSPACE =>
        let {newString, newCursorPosition} =
          removeCharacterBefore(text, cursorPosition);
        onChange(newString, newCursorPosition);

      | Key.KEY_ESCAPE =>
        onKeyDown(event);
        Focus.loseFocus();

      | _ =>
        onKeyDown(event)
      };
    };

    /*
       computed styles
     */

    let allStyles =
      Style.(
        merge(
          ~source=[
            flexDirection(`Row),
            alignItems(`Center),
            justifyContent(`FlexStart),
            overflow(`Hidden),
            cursor(MouseCursors.text),
            ...defaultStyles,
          ],
          ~target=style,
        )
      );

    let viewStyles = Style.extractViewStyles(allStyles);
    let inputFontSize = fontSize;
    let inputColor = Selector.select(style, Color, Colors.black);
    let inputFontFamily =
      Selector.select(style, FontFamily, "Roboto-Regular.ttf");

    let cursorOpacity = 1.0;

    let cursor = {
      let (startStr, _) =
        getStringParts(cursorPosition + String.length(prefix), valueToDisplay);
      let dimension =
        Revery_Draw.Text.measure(
          ~fontFamily=inputFontFamily,
          ~fontSize=inputFontSize,
          startStr,
        );
      <View
        style=Style.[
          position(`Absolute),
          marginLeft(dimension.width + inputTextMargin + 1),
          marginTop((defaultHeight - dimension.height) / 2),
        ]>
        <Opacity opacity=cursorOpacity>
          <Container width=2 height=inputFontSize color=cursorColor />
        </Opacity>
      </View>;
    };

    let makeTextComponent = content =>
      <Text
        text=content
        style=Style.[
          color(showPlaceholder ? placeholderColor : inputColor),
          fontFamily(inputFontFamily),
          fontSize(inputFontSize),
          alignItems(`Center),
          justifyContent(`FlexStart),
          marginLeft(inputTextMargin),
        ]
      />;

    let textView =
        makeTextComponent(showPlaceholder ? placeholder : valueToDisplay);

    /*
       component
     */
    (
      hooks,
      <Clickable
        componentRef={autofocus ? Focus.focus : ignore}
        onKeyDown=handleKeyDown
        onKeyPress=handleKeyPress>
        <View style=viewStyles>
          cursor
          textView
        </View>
      </Clickable>,
    );
  });

let createElement =
    (
      ~children as _,
      ~style=defaultStyles,
      ~placeholderColor=Colors.grey,
      ~cursorColor=Colors.black,
      ~autofocus=false,
      ~placeholder="",
      ~prefix="",
      ~fontSize=14,
      ~onChange=(_, _) => (),
      ~cursorPosition,
      ~text,
      (),
    ) =>
  make(
    ~style,
    ~placeholder,
    ~prefix,
    ~autofocus,
    ~cursorColor,
    ~fontSize,
    ~placeholderColor,
    ~onChange,
    ~cursorPosition,
    ~text,
    (),
  );
