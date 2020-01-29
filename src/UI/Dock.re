open Revery.UI;

open Oni_Model;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;

let button = Style.[marginVertical(24)];

let onExplorerClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(FileExplorerClick));
};

let onSearchClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(SearchClick));
};

let onExtensionsClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(ExtensionsClick));
};

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(150))
    |> ease(Easing.ease)
    |> tween(-50.0, 0.)
    |> delay(Revery.Time.milliseconds(75))
  );

let%component make = (~state: State.t, ()) => {
  let bg = state.theme.activityBarBackground;
  let fg = state.theme.activityBarForeground;

  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

  <View
    style=Style.[
      top(0),
      bottom(0),
      backgroundColor(bg),
      alignItems(`Center),
      width(50),
      transform(Transform.[TranslateX(transition)]),
    ]>
    <Sneakable onClick=onExplorerClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.file />
    </Sneakable>
    <Sneakable onClick=onSearchClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.search />
    </Sneakable>
    <Sneakable onClick=onExtensionsClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.box />
    </Sneakable>
  </View>;
};
