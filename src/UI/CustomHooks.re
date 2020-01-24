open Revery;
open Revery.UI;

let colorTransition =
    (
      ~duration=Time.seconds(1),
      ~delay as delayDuration=Time.zero,
      ~easing=Easing.linear,
      target,
    ) => {
  let%hook ((start, stop), setTarget) = Hooks.state((target, target));

  let gradient = (value: Animation.NormalizedTime.t) =>
    Color.mix(~start, ~stop, ~amount=(value :> float));

  let anim =
    Animation.(
      animate(duration)
      |> delay(delayDuration)
      |> ease(easing)
      |> map(gradient)
    );

  let%hook (current, _animationState, resetTimer) = Hooks.animation(anim);

  let%hook () =
    Hooks.effect(
      If((!=), target),
      () => {
        resetTimer();
        setTarget(_ => (current, target));
        None;
      },
    );

  current;
};
