open Revery;
open Revery.UI;
open Oni_Model;

type action =
  | Tick(Time.t)
  | Pause
  | Resume;

let reducer =
  fun
  | Tick(dt) => Animation.tick(Time.toFloatSeconds(dt))
  | Pause => Animation.pause
  | Resume => Animation.resume;

let%component make =
              (
                ~children as renderFunc: float => React.element(React.node),
                ~isActive=true,
                ~duration=?,
                ~repeat=?,
                ~delay=?,
                (),
              ) => {
  let%hook (animation, dispatch) =
    Hooks.reducer(
      ~initialState=
        Animation.create(~isActive, ~duration?, ~repeat?, ~delay?, ()),
      reducer,
    );

  let%hook () =
    Hooks.effect(
      If((!=), isActive),
      () => {
        dispatch(isActive ? Resume : Pause);
        None;
      },
    );

  let%hook () = Hooks.tick(~tickRate=Time.zero, dt => dispatch(Tick(dt)));

  renderFunc(Animation.getValue(animation));
};
