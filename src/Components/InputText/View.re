open Oni_Core;

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Sneakable = Feature_Sneak.View.Sneakable;

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
      Hooks.effect(OnMountAndIf((!=), isFocused), () =>
        if (isFocused) {
          let clear =
            Tick.interval(
              ~name="Input Text Cursor Interval",
              time => dispatch(Tick(time)),
              Time.ms(16),
            );
          Some(() => {clear()});
        } else {
          None;
        }
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

module Constants = {
  let cursorWidth = 2;
  let selectionOpacity = 0.75;
};

module Colors = {
  let color = key => {
    let key = ColorTheme.key(key);

    theme =>
      ColorTheme.Colors.get(key, theme)
      |> Option.value(~default=Colors.transparentBlack);
  };

  let foreground = color("input.foreground");
  let background = color("input.background");
  let border = color("input.border");
  let focusBorder = color("focusBorder");
  let placeholderForeground = color("input.placeholderForeground");
  let selection = color("selection.background");
};

module Styles = {
  open Style;
  let default = (~shadowOpacity, ~theme, ~focused, ()) =>
    Style.[
      color(Colors.foreground(theme)),
      paddingVertical(8),
      paddingHorizontal(12),
      border(
        ~width=1,
        ~color=focused ? Colors.focusBorder(theme) : Colors.border(theme),
      ),
      backgroundColor(Colors.background(theme)),
      boxShadow(
        ~xOffset=4.,
        ~yOffset=4.,
        ~blurRadius=focused ? 8. : 0.,
        ~spreadRadius=0.,
        ~color=Color.rgba(0., 0., 0., shadowOpacity),
      ),
    ];

  let _all = (~shadowOpacity, ~theme, focused) => [
    flexDirection(`Row),
    alignItems(`Center),
    justifyContent(`FlexStart),
    cursor(MouseCursors.text),
    ...default(~shadowOpacity, ~theme, ~focused, ()),
  ];

  let box = (~shadowOpacity, ~theme, ~focused) => {
    _all(~shadowOpacity, ~theme, focused);
  };

  let marginContainer = [
    flexDirection(`Row),
    alignItems(`Center),
    justifyContent(`FlexStart),
    flexGrow(1),
    overflow(`Hidden),
  ];

  let cursor = offset => [
    position(`Absolute),
    marginTop(2),
    transform(Transform.[TranslateX(float(offset))]),
  ];

  let selection = offset => [
    position(`Absolute),
    marginTop(2),
    transform(Transform.[TranslateX(float(offset))]),
  ];

  let textContainer = [flexGrow(1), overflow(`Hidden)];

  let text = (~showPlaceholder, ~placeholderColor, ~textColor, ~scrollOffset) => [
    color(showPlaceholder ? placeholderColor : textColor),
    alignItems(`Center),
    justifyContent(`FlexStart),
    textWrap(TextWrapping.NoWrap),
    transform(Transform.[TranslateX(float(- scrollOffset^))]),
  ];
};

let%component make =
              (
                ~model: Model.t,
                ~theme,
                ~fontSize=18.,
                ~shadowOpacity=0.5,
                ~fontFamily=Revery.Font.Family.fromFile("Roboto-Regular.ttf"),
                ~placeholderColor=Colors.placeholderForeground(theme),
                ~cursorColor=Colors.foreground(theme),
                ~selectionColor=Colors.selection(theme),
                ~prefix="",
                ~isFocused,
                ~dispatch,
                (),
              ) => {
  let%hook textRef = Hooks.ref(None);
  let%hook scrollOffset = Hooks.ref(0);

  let%hook () =
    Hooks.effect(
      OnMountAndIf((!=), isFocused),
      () => {
        if (isFocused) {
          dispatch(Model.GainedFocus);
        } else {
          dispatch(LostFocus);
        };

        Some(() => dispatch(LostFocus));
      },
    );

  let {placeholder, value, selection, _}: Model.t = model;
  let displayValue = prefix ++ value;
  let showPlaceholder = displayValue == "";

  let measureTextWidth = text => {
    let dimensions =
      Revery_Draw.Text.dimensions(
        ~smoothing=Revery.Font.Smoothing.default,
        ~fontFamily,
        ~fontWeight=Normal,
        ~fontSize,
        text,
      );

    dimensions.width;
  };

  let%hook (cursorOpacity, resetCursor) =
    Cursor.use(~interval=Time.ms(500), ~isFocused);

  let%hook () =
    Hooks.effect(
      If((!=), (value, selection, isFocused)),
      () => {
        resetCursor();
        None;
      },
    );

  let () = {
    let cursorOffset =
      measureTextWidth(Zed_utf8.sub(displayValue, 0, selection.focus))
      |> int_of_float;

    switch (Option.bind(textRef^, r => r#getParent())) {
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
        if (i > Zed_utf8.length(value)) {
          i - 1;
        } else {
          let width =
            measureTextWidth(Zed_utf8.sub(value, 0, i)) |> int_of_float;

          if (width > offset) {
            let isCurrentNearest = width - offset < offset - last;
            isCurrentNearest ? i : i - 1;
          } else {
            loop(i + 1, width);
          };
        };

      loop(1, 0);
    };

    switch (textRef^) {
    | Some(node) =>
      let offset =
        int_of_float(event.mouseX) - offsetLeft(node) + scrollOffset^;
      let nearestOffset = indexNearestOffset(offset);
      let selection = Selection.collapsed(~text=value, nearestOffset);
      resetCursor();
      dispatch(Model.Clicked({selection: selection}));

    | None => ()
    };
  };

  let onSneak = () => {
    dispatch(Model.Sneaked);
  };

  let cursor = () => {
    let startStr =
      Zed_utf8.before(
        displayValue,
        selection.focus + Zed_utf8.length(prefix),
      );

    let textWidth = measureTextWidth(startStr) |> int_of_float;

    let offset = textWidth - scrollOffset^;

    <View style={Styles.cursor(offset)}>
      <Opacity opacity=cursorOpacity>
        <Container
          width=Constants.cursorWidth
          height={fontSize |> int_of_float}
          color=cursorColor
        />
      </Opacity>
    </View>;
  };

  let selectionView = () =>
    if (Selection.isCollapsed(selection)) {
      React.empty;
    } else {
      let startOffset = Selection.offsetLeft(selection);
      let endOffset = Selection.offsetRight(selection);

      let beginnigStartStr =
        Zed_utf8.before(displayValue, startOffset + Zed_utf8.length(prefix));
      let beginningTextWidth =
        measureTextWidth(beginnigStartStr) |> int_of_float;
      let startOffset = beginningTextWidth - scrollOffset^;

      let endingStartStr =
        Zed_utf8.before(displayValue, endOffset + Zed_utf8.length(prefix));
      let endingTextWidth = measureTextWidth(endingStartStr) |> int_of_float;
      let endOffset = endingTextWidth - scrollOffset^;
      let width = endOffset - startOffset + Constants.cursorWidth;

      <View style={Styles.selection(startOffset)}>
        <Opacity opacity=Constants.selectionOpacity>
          <Container
            width
            height={fontSize |> int_of_float}
            color=selectionColor
          />
        </Opacity>
      </View>;
    };

  let textColor = Colors.foreground(theme);

  let text = () =>
    <Text
      ref={node => textRef := Some(node)}
      text={showPlaceholder ? placeholder : displayValue}
      style={Styles.text(
        ~showPlaceholder,
        ~placeholderColor,
        ~textColor,
        ~scrollOffset,
      )}
      fontFamily
      fontSize
    />;

  <Sneakable sneakId="text" onAnyClick=handleClick onSneak>
    <View
      style={Styles.box(~shadowOpacity, ~theme, ~focused=model.isFocused)}>
      <View style=Styles.marginContainer>
        <selectionView />
        <cursor />
        <View style=Styles.textContainer> <text /> </View>
      </View>
    </View>
  </Sneakable>;
};
