open Revery;
open Revery.UI;
open Revery.UI.Components;

type state = {
  isFocused: bool,
  internalValue: string,
  cursorPosition: int,
};

type action =
  | CursorLeft
  | CursorRight
  | SetFocus(bool)
  | DeleteCharacter
  | DeleteLine
  | DeleteWord
  | Backspace
  | InsertText(string);

type textUpdate = {
  newString: string,
  cursorPosition: int,
};

let getStringParts = (index, str) => {
  switch (index) {
  | 0 => ("", str)
  | _ =>
    let strBeginning =
      try (Str.string_before(str, index)) {
      | _ => str
      };
    let strEnd =
      try (Str.string_after(str, index)) {
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
  {newString, cursorPosition: nextPosition};
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
  {newString, cursorPosition};
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
  {newString: beforeStr ++ afterStr, cursorPosition: positionToDeleteTo};
};

let addCharacter = (word, char, index) => {
  let (startStr, endStr) = getStringParts(index, word);
  {
    newString: startStr ++ char ++ endStr,
    cursorPosition: String.length(startStr) + 1,
  };
};
let reducer = (action, state) =>
  switch (action) {
  | SetFocus(isFocused) => {...state, isFocused}
  | CursorLeft => {
      ...state,
      cursorPosition:
        getSafeStringBounds(state.internalValue, state.cursorPosition, -1),
    }
  | CursorRight => {
      ...state,
      cursorPosition:
        getSafeStringBounds(state.internalValue, state.cursorPosition, 1),
    }
  | DeleteWord =>
    let {newString, cursorPosition} =
      deleteWord(state.internalValue, state.cursorPosition);
    {...state, internalValue: newString, cursorPosition};
  | DeleteLine => {...state, internalValue: "", cursorPosition: 0}
  | DeleteCharacter =>
    let {newString, cursorPosition} =
      removeCharacterAfter(state.internalValue, state.cursorPosition);
    {...state, internalValue: newString, cursorPosition};
  | Backspace =>
    let {newString, cursorPosition} =
      removeCharacterBefore(state.internalValue, state.cursorPosition);
    {...state, internalValue: newString, cursorPosition};
  | InsertText(t) =>
    let {newString, cursorPosition} =
      addCharacter(state.internalValue, t, state.cursorPosition);
    {...state, internalValue: newString, cursorPosition};
  };

let defaultHeight = 50;
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
      ~autofocus,
      ~placeholder,
      ~cursorColor,
      ~placeholderColor,
      ~onChange,
      ~onKeyDown,
      (),
    ) =>
  component(slots => {
    let (state, dispatch, slots) =
      Hooks.reducer(
        ~initialState={
          internalValue: "",
          cursorPosition: 0,
          isFocused: false,
        },
        reducer,
        slots,
      );

    let valueToDisplay = state.internalValue;

    let slots =
      Hooks.effect(
        If((!=), valueToDisplay),
        () => {
          onChange(valueToDisplay);
          None;
        },
        slots,
      );

    let handleKeyPress = (event: NodeEvents.keyPressEventParams) => {
      dispatch(InsertText(event.character));
    };

    let handleKeyDown = (event: NodeEvents.keyEventParams) => {
      switch (event.key) {
      | Key.KEY_LEFT =>
        onKeyDown(event);
        dispatch(CursorLeft);
      | Key.KEY_RIGHT =>
        onKeyDown(event);
        dispatch(CursorRight);
      | Key.KEY_H when event.ctrlKey => dispatch(Backspace)
      | Key.KEY_U when event.ctrlKey => dispatch(DeleteLine)
      | Key.KEY_W when event.ctrlKey => dispatch(DeleteWord)
      | Key.KEY_DELETE => dispatch(DeleteCharacter)
      | Key.KEY_BACKSPACE => dispatch(Backspace)
      | Key.KEY_ESCAPE =>
        onKeyDown(event);
        Focus.loseFocus();
      | _ => onKeyDown(event)
      };
    };

    let hasPlaceholder = String.length(valueToDisplay) < 1;

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
    let inputFontSize = Selector.select(style, FontSize, 18);
    let inputColor = Selector.select(style, Color, Colors.black);
    let inputFontFamily =
      Selector.select(style, FontFamily, "Roboto-Regular.ttf");

    let cursorOpacity = 1.0;

    let cursor = {
      let (startStr, _) =
        getStringParts(state.cursorPosition, valueToDisplay);
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
          color(hasPlaceholder ? placeholderColor : inputColor),
          fontFamily(inputFontFamily),
          fontSize(inputFontSize),
          alignItems(`Center),
          justifyContent(`FlexStart),
          marginLeft(inputTextMargin),
        ]
      />;

    let placeholderText = makeTextComponent(placeholder);
    let inputText = makeTextComponent(valueToDisplay);

    /*
       component
     */
    (
      slots,
      <Clickable
        onFocus={() => dispatch(SetFocus(true))}
        onBlur={() => dispatch(SetFocus(false))}
        componentRef={autofocus ? Focus.focus : ignore}
        onKeyDown=handleKeyDown
        onKeyPress=handleKeyPress>
        <View style=viewStyles>
          cursor
          {hasPlaceholder ? placeholderText : inputText}
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
      ~onChange=_ => (),
      (),
    ) =>
  make(
    ~style,
    ~placeholder,
    ~autofocus,
    ~cursorColor,
    ~placeholderColor,
    ~onChange,
    (),
  );
