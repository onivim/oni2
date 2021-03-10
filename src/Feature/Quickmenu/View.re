open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Components;

open Model;

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

let component = React.Expert.component("Feature_Quickmenu.View");

let make =
    (
      ~font: UiFont.t,
      ~theme,
      ~config: Oni_Core.Config.resolver,
      ~model: Model.model(_),
      ~dispatch,
      (),
    ) =>
  component(hooks => {
    let current = Model.current(model) |> Option.get;
    let filterProgress = Complete;
    let ripgrepProgress = Complete;
    let focused = current |> Schema.Instance.focused;
    let inputText = current |> Schema.Instance.text;
    let prefix = None;

    let progress =
      Model.(
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

    let count = Schema.Instance.count(current);

    let onFocusedChange = index => {
      dispatch(Model.ItemFocused(index));
    };

    let onSelect = index => {
      dispatch(Model.ItemSelected(index));
    };

    let renderItem = index => {
      switch (Schema.Instance.render(~index, ~theme, ~font, current)) {
      | None => React.empty
      | Some(elem) =>
        let isFocused = Some(index) == focused;
        <MenuItem
          onClick={() => onSelect(index)}
          theme
          label={`Custom(elem)}
          onMouseOver={() => onFocusedChange(index)}
          isFocused
        />;
      };
    };

    let input = () =>
      <View style=Styles.inputContainer>
        <Component_InputText.View
          ?prefix
          shadowOpacity=0.1
          fontFamily={font.family}
          fontSize=14.
          isFocused=true
          dispatch={msg => dispatch(Model.Input(msg))}
          model=inputText
          theme
        />
      </View>;

    let dropdown = () =>
      <View style={Styles.dropdown(~numItems=count)}>
        <FlatList rowHeight=Constants.rowHeight count focused theme>
          ...renderItem
        </FlatList>
      </View>;

    let onClickBackground = () => {
      dispatch(Model.BackgroundClicked);
    };

    let innerClicked = ref(false);

    // TODO: Bring back oni box shadow
    (
      <View
        style=Style.[
          position(`Absolute),
          top(0),
          left(0),
          right(0),
          bottom(0),
          backgroundColor(Revery.Color.rgba(0., 0., 0., 0.2)),
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
          theme
          config
          onMouseDown={_ => {innerClicked := true}}
          style={Styles.container(theme)}>
          <input />
          <dropdown />
          {switch (progress) {
           | Complete => <progressBar progress=0. theme /> // TODO: SHould be REact.empty, but a reconciliation bug then prevents the progress bar from rendering
           | InProgress(progress) => <progressBar progress theme />
           | Loading => <busyBar theme />
           }}
        </OniBoxShadow>
      </View>,
      hooks,
    );
  });
