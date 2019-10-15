open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;


type action =
  | Tick(Time.t)
  | Pause
  | Resume;

let reducer = fun
  | Tick(dt) => Animation.tick(Time.toSeconds(dt))
  | Pause => Animation.pause
  | Resume => Animation.resume;

let component = React.component("AnimatedView");
let createElement =
    (
      ~children as renderFunc: float => React.syntheticElement,
      ~isActive=true,
      ~duration=?,
      ~repeat=?,
      ~delay=?,
      (),
    ) =>
  component(hooks => {
    let (animation, dispatch, hooks) =
      Hooks.reducer(
        Animation.create(~isActive, ~duration?, ~repeat?, ~delay?, ()),
        reducer,
        hooks
      );

    let hooks =
      Hooks.effect(
        If((!=), isActive),
        () => {
          Printf.printf("-- %b\n%!", isActive);
          dispatch(isActive ? Resume : Pause);
          None
        }, hooks);
    
    let hooks =
      Hooks.tick(~tickRate=Time.Seconds(0.), dt => dispatch(Tick(dt)), hooks);

    (
      hooks,
      renderFunc(Animation.getValue(animation))
    );
  });
