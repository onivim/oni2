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

let useExpiration = (~equals=(==), ~expireAfter, items) => {
  let%hook (active, setActive) = Hooks.state([]);
  let%hook expired = Hooks.ref([]);
  let%hook (time, _reset) = Hooks.timer(~active=active != [], ());

  let (stillActive, freshlyExpired) =
    List.partition(
      ((_item, activated)) => Time.(time - activated < expireAfter),
      active,
    );

  if (freshlyExpired != []) {
    setActive(_ => stillActive);

    expired :=
      freshlyExpired
      |> List.map(((item, _t)) => item)
      |> List.rev_append(expired^);
  };

  let%hook () =
    Hooks.effect(
      If((!==), items),
      () => {
        let untracked =
          items
          |> List.filter(item => !List.exists(equals(item), expired^))
          |> List.filter(item =>
               !List.exists(((it, _t)) => equals(it, item), active)
             );

        if (untracked != []) {
          let init = item => (item, time);
          setActive(tracked => List.map(init, untracked) @ tracked);
        };

        // TODO: Garbage collection of expired, but on what condition?

        None;
      },
    );

  List.map(((item, _t)) => item, stillActive);
};
