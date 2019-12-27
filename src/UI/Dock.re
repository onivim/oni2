open Revery.UI;
open Revery.UI.Components;

open Oni_Model;

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
    <Clickable onClick=onExplorerClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.file />
    </Clickable>
    <Clickable onClick=onSearchClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.search />
    </Clickable>
    <Clickable onClick=onExtensionsClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.box />
    </Clickable>
  </View>;
};
