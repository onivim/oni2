open Revery;
open Revery.UI;

let%component make =
              (
                ~children as renderFunc: float => React.element(React.node),
                ~isActive=true,
                ~duration=Time.seconds(1),
                ~repeat=false,
                ~delay=Time.zero,
                (),
              ) => {
  let animation =
    Animation.animate(duration)
    |> (repeat ? Animation.repeat : (x => x))
    |> Animation.delay(delay)
    |> Animation.tween(0., 1.);

  let%hook (value, _, _) = Hooks.animation(~active=isActive, animation);

  renderFunc(value);
};
