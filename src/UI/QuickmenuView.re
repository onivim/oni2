open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;
open Oni_Components;

module Colors = Feature_Theme.Colors;

module Constants = {
  let menuWidth = 400;
  let menuHeight = 320;
  let rowHeight = 40;
};

module Styles = {
  open Style;

  let container = theme => [
    backgroundColor(Colors.Menu.background.from(theme)),
    color(Colors.Menu.foreground.from(theme)),
    width(Constants.menuWidth),
  ];

  let inputContainer = [padding(5)];

  let dropdown = (~numItems) => [
    height(
      Constants.rowHeight * numItems > Constants.menuHeight
        ? Constants.menuHeight : Constants.rowHeight * numItems,
    ),
    overflow(`Hidden),
  ];

  let menuItem = [cursor(Revery.MouseCursors.pointer)];

  let label = (~theme, ~highlighted) => [
    textOverflow(`Ellipsis),
    color(
      highlighted
        ? Colors.Oni.normalModeBackground.from(theme)
        : Colors.Menu.foreground.from(theme),
    ),
    textWrap(TextWrapping.NoWrap),
  ];

  let progressBarTrack = [height(2), overflow(`Hidden)];

  let progressBarIndicator = (~width, ~offset, ~theme) => [
    height(2),
    Style.width(width),
    transform(Transform.[TranslateX(offset)]),
    backgroundColor(Colors.Oni.normalModeBackground.from(theme)),
  ];
};

let onFocusedChange = index =>
  GlobalContext.current().dispatch(ListFocus(index));

let onSelect = _ => GlobalContext.current().dispatch(ListSelect);

let progressBar = (~progress, ~theme, ()) => {
  let indicator = () => {
    let width = int_of_float(float(Constants.menuWidth) *. progress);
    let offset = 0.;

    <View style={Styles.progressBarIndicator(~width, ~offset, ~theme)} />;
  };

  <View style=Styles.progressBarTrack> <indicator /> </View>;
};

let%component busyBar = (~theme, ()) => {
  let%hook (time, _, _) =
    Animation.(
      animate(Time.ms(1500))
      |> ease(Easing.easeInOut)
      |> repeat
      |> tween(0., 1.)
      |> Hooks.animation(~name="QuickMenu busyBar")
    );

  let indicator = () => {
    let width = 100;
    let trackWidth = float(Constants.menuWidth + width);
    let offset = trackWidth *. Easing.easeInOut(time) -. float(width);

    <View style={Styles.progressBarIndicator(~width, ~offset, ~theme)} />;
  };

  <View style=Styles.progressBarTrack> <indicator /> </View>;
};

let make =
    (
      ~font: UiFont.t,
      ~theme,
      ~configuration: Configuration.t,
      ~state: Quickmenu.t,
      //      ~placeholder: string="type here to search the menu",
      ~onFocusedChange: int => unit=onFocusedChange,
      ~onSelect: int => unit=onSelect,
      (),
    ) => {
  let Quickmenu.{
        items,
        filterProgress,
        ripgrepProgress,
        focused,
        inputText,
        prefix,
        variant,
        _,
      } = state;

  let progress =
    Actions.(
      switch (filterProgress, ripgrepProgress) {
      // Evaluate ripgrep progress first, because it could still be processing jobs
      // while the filter job is 'complete'.
      | (_, Loading)
      | (Loading, _) => Loading
      | (InProgress(a), InProgress(b)) => InProgress((a +. b) /. 2.)

      | (InProgress(value), _)
      | (_, InProgress(value)) => InProgress(value)

      | (Complete, Complete) => Complete
      }
    );

  let renderItem = index => {
    let item = items[index];
    let isFocused = Some(index) == focused;

    let style = Styles.label(~theme);
    let text = Quickmenu.getLabel(item);
    let highlights = item.highlight;
    let normalStyle = style(~highlighted=false);
    let highlightStyle = style(~highlighted=true);
    let labelView =
      <HighlightText
        fontFamily={font.family}
        fontSize=12.
        style=normalStyle
        highlightStyle
        text
        highlights
      />;

    <MenuItem
      onClick={() => onSelect(index)}
      theme
      style=Styles.menuItem
      label={`Custom(labelView)}
      icon={item.icon}
      font
      fontSize=14.
      onMouseOver={() => onFocusedChange(index)}
      isFocused
    />;
  };

  let input = () =>
    <View style=Styles.inputContainer>
      <Component_InputText.View
        ?prefix
        fontFamily={font.family}
        fontSize=14.
        isFocused=true
        dispatch={msg =>
          GlobalContext.current().dispatch(
            Actions.QuickmenuInputMessage(msg),
          )
        }
        model=inputText
        theme
      />
    </View>;

  let dropdown = () =>
    <View style={Styles.dropdown(~numItems=Array.length(items))}>
      <FlatList
        rowHeight=Constants.rowHeight
        count={Array.length(items)}
        focused
        theme>
        ...renderItem
      </FlatList>
    </View>;

  <AllowPointer>
    <OniBoxShadow configuration theme>
      <View style={Styles.container(theme)}>
        {switch (variant) {
         | EditorsPicker => React.empty
         | _ => <input />
         }}
        {switch (variant) {
         | Wildmenu(SearchForward | SearchReverse) => React.empty
         | _ => <dropdown />
         }}
        {switch (progress) {
         | Complete => <progressBar progress=0. theme /> // TODO: SHould be REact.empty, but a reconciliation bug then prevents the progress bar from rendering
         | InProgress(progress) => <progressBar progress theme />
         | Loading => <busyBar theme />
         }}
      </View>
    </OniBoxShadow>
  </AllowPointer>;
};
