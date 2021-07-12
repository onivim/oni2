open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;
open Oni_Components;

module Colors = Feature_Theme.Colors;

module Clickable = Revery.UI.Components.Clickable;

module Constants = {
  let menuWidth = 600;
  let menuHeight = 320;
  let rowHeight = 40;
};

module Styles = {
  open Style;

  let container = theme => [
    backgroundColor(Colors.Menu.background.from(theme)),
    color(Colors.Menu.foreground.from(theme)),
    width(Constants.menuWidth),
    marginTop(25),
    //pointerEvents(`Ignore),
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

let onSelect = _ =>
  GlobalContext.current().dispatch(
    ListSelect({direction: Oni_Core.SplitDirection.Current}),
  );

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
      ~config: Oni_Core.Config.resolver,
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

  let defaultRenderer = (normalStyle, highlightStyle, text, highlights) => {
    <HighlightText
      fontFamily={font.family}
      fontSize=12.
      style=normalStyle
      highlightStyle
      text
      highlights
    />;
  };

  let fileRenderer = (normalStyle, highlightStyle, fullPath, highlights) => {
    // Try and get basename from path

    let fileName = Filename.basename(fullPath);
    let len = String.length(fullPath);
    let fileNameLen = String.length(fileName);

    let parentDirectoryLength = len - fileNameLen;
    let parentDirectory =
      String.sub(fullPath, 0, len - fileNameLen)
      |> Utility.Path.trimTrailingSeparator;

    let fileNameHighlights =
      highlights
      |> List.filter_map(((low, high)) =>
           if (low <= parentDirectoryLength && high <= parentDirectoryLength) {
             None;
           } else if (low <= parentDirectoryLength) {
             Some((0, high - parentDirectoryLength));
           } else {
             Some((
               low - parentDirectoryLength,
               high - parentDirectoryLength,
             ));
           }
         );

    let parentHighlights =
      highlights
      |> List.filter_map(((low, high)) =>
           if (low >= parentDirectoryLength) {
             None;
           } else if (high >= parentDirectoryLength) {
             Some((low, parentDirectoryLength));
           } else {
             Some((low, high));
           }
         );

    <View style=Style.[flexDirection(`Row)]>
      //alignItems(`Center)

        <View style=Style.[flexShrink(0), marginTop(2), marginRight(8)]>
          <HighlightText
            fontFamily={font.family}
            fontSize=12.
            style=normalStyle
            highlightStyle
            text=fileName
            highlights=fileNameHighlights
          />
        </View>
        <View
          style=Style.[
            flexShrink(1),
            marginTop(2),
            opacity(0.75),
            transform([Transform.TranslateY(1.)]),
          ]>
          <HighlightText
            fontFamily={font.family}
            fontSize=11.
            style=normalStyle
            highlightStyle
            text=parentDirectory
            highlights=parentHighlights
          />
        </View>
      </View>;
  };

  let renderer =
    switch (variant) {
    | FilesPicker => fileRenderer
    | _ => defaultRenderer
    };

  let renderItem = index => {
    let item = items[index];
    let isFocused = Some(index) == focused;

    let style = Styles.label(~theme);
    let text = Quickmenu.getLabel(item);
    let highlights = item.highlight;
    let normalStyle = style(~highlighted=false);
    let highlightStyle = style(~highlighted=true);
    let labelView = renderer(normalStyle, highlightStyle, text, highlights);

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

  let onClickBackground = () => {
    GlobalContext.current().dispatch(Actions.QuickmenuClose);
  };

  let innerClicked = ref(false);

  <View
    style=Style.[
      position(`Absolute),
      top(0),
      left(0),
      right(0),
      bottom(0),
      pointerEvents(`Allow),
      alignItems(`Center),
    ]
    onMouseDown={_ =>
      // HACK: This callback would be called last, after the 'inner' callback being called.
      // We don't want to execute the `onClickBackground` function if we're clicking inside the menu.
      // A better mechanism would be a robust way to stop propagationnn of the mouse event.

        if (! innerClicked^) {
          onClickBackground();
        }
      }>
    <OniBoxShadow
      onMouseDown={_ => {innerClicked := true}}
      style={Styles.container(theme)}
      config
      theme>
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
    </OniBoxShadow>
  </View>;
};
