open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;

module Constants = {
  let menuWidth = 400;
  let menuHeight = 320;
};

module Styles = {
  let container = (theme: Theme.t) =>
    Style.[
      backgroundColor(theme.menuBackground),
      color(theme.menuForeground),
    ];

  let input = font =>
    Style.[
      border(~width=2, ~color=Color.rgba(0., 0., 0., 0.1)),
      backgroundColor(Color.rgba(0., 0., 0., 0.3)),
      color(Colors.white),
      fontFamily(font),
      fontSize(14),
    ];

  let dropdown = Style.[height(Constants.menuHeight), overflow(`Hidden)];

  let menuItem = Style.[fontSize(14), cursor(Revery.MouseCursors.pointer)];

  let label = (~font: UiFont.t, ~theme: Theme.t, ~highlighted, ~isFocused) =>
    Style.[
      fontFamily(font.fontFile),
      textOverflow(`Ellipsis),
      fontSize(12),
      backgroundColor(
        isFocused ? theme.menuSelectionBackground : theme.menuBackground,
      ),
      color(
        highlighted ? theme.oniNormalModeBackground : theme.menuForeground,
      ),
      textWrap(TextWrapping.NoWrap),
    ];

  let progressBarTrack = Style.[height(2), overflow(`Hidden)];

  let progressBarIndicator = (~width as barWidth, ~offset, ~theme: Theme.t) =>
    Style.[
      height(2),
      width(barWidth),
      transform(Transform.[TranslateX(offset)]),
      backgroundColor(theme.oniNormalModeBackground),
    ];
};

let onFocusedChange = index =>
  GlobalContext.current().dispatch(ListFocus(index));

let onInput = (text, cursorPosition) =>
  GlobalContext.current().dispatch(QuickmenuInput({text, cursorPosition}));

let onSelect = _ => GlobalContext.current().dispatch(ListSelect);

let progressBar = (~progress=?, ~theme, ()) => {
  let indicatorWidth = 100.;
  let menuWidth = float_of_int(Constants.menuWidth);
  let trackWidth = menuWidth +. indicatorWidth;
  <AnimatedView duration=1.5 repeat=true>
    ...{t => {
      let (width, offset) =
        switch (progress) {
        | Some(progress) =>
          // Determinate
          (int_of_float(menuWidth *. progress), 0.)

        | None =>
          // Indeterminate
          (
            int_of_float(indicatorWidth),
            trackWidth *. Easing.easeInOut(t) -. indicatorWidth,
          )
        };

      <View style=Styles.progressBarTrack>
        <View style={Styles.progressBarIndicator(~width, ~offset, ~theme)} />
      </View>;
    }}
  </AnimatedView>;
};

let make =
    (
      ~font: UiFont.t,
      ~theme: Theme.t,
      ~configuration: Configuration.t,
      ~autofocus: bool=true,
      ~state: Quickmenu.t,
      ~placeholder: string="type here to search the menu",
      ~onInput: (string, int) => unit=onInput,
      ~onFocusedChange: int => unit=onFocusedChange,
      ~onSelect: int => unit=onSelect,
      (),
    ) => {
  let Quickmenu.{
        items,
        filterProgress,
        ripgrepProgress,
        focused,
        query,
        cursorPosition,
        prefix,
        _,
      } = state;

  let progress =
    Actions.(
      switch (filterProgress, ripgrepProgress) {
      | (Loading, _)
      | (_, Loading) => Loading

      | (InProgress(a), InProgress(b)) => InProgress((a +. b) /. 2.)

      | (InProgress(value), _)
      | (_, InProgress(value)) => InProgress(value)

      | (Complete, Complete) => Complete
      }
    );

  let renderItem = index => {
    let item = items[index];
    let isFocused = Some(index) == focused;

    let style = Styles.label(~font, ~theme, ~isFocused);
    let text = Quickmenu.getLabel(item);
    let highlights = item.highlight;
    let normalStyle = style(~highlighted=false);
    let highlightStyle = style(~highlighted=true);
    let labelView =
      <HighlightText style=normalStyle highlightStyle text highlights />;

    <MenuItem
      onClick={() => onSelect(index)}
      theme
      style=Styles.menuItem
      label={`Custom(labelView)}
      icon={item.icon}
      onMouseOver={() => onFocusedChange(index)}
      isFocused
    />;
  };

  <AllowPointer>
    <OniBoxShadow configuration theme>
      <View style={Styles.container(theme)}>
        <View style=Style.[width(Constants.menuWidth), padding(5)]>
          <OniInput
            autofocus
            placeholder
            ?prefix
            cursorColor=Colors.white
            style={Styles.input(font.fontFile)}
            onChange=onInput
            value=query
            cursorPosition
          />
        </View>
        <View style=Styles.dropdown>
          <FlatList
            rowHeight=40
            count={Array.length(items)}
            focused
            render=renderItem
          />
          {switch (progress) {
           | Complete => React.empty
           | InProgress(progress) => <progressBar progress theme />
           | Loading => <progressBar theme />
           }}
        </View>
      </View>
    </OniBoxShadow>
  </AllowPointer>;
};
