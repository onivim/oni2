module Sub = {
  type timerOnceParams = {
    delay: Revery.Time.t,
    uniqueId: string,
  };

  module TimerOnceSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = Revery.Time.t;
      type nonrec params = timerOnceParams;
      type state = unit => unit;

      let name = "Service_Time.once";
      let id = ({uniqueId, _}) => uniqueId;

      let init = (~params, ~dispatch) => {
        Revery.Tick.timeout(
          ~name="Service_Time.once: " ++ params.uniqueId,
          () => {dispatch(Revery.Time.now())},
          params.delay,
        );
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state) => {
        state();
      };
    });

  let once = (~uniqueId: string, ~delay, ~msg) => {
    TimerOnceSubscription.create({delay, uniqueId})
    |> Isolinear.Sub.map(current => msg(~current));
  };

  type timerIntervalParams = {
    every: Revery.Time.t,
    uniqueId: string,
  };

  module TimerIntervalSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = Revery.Time.t;
      type nonrec params = timerIntervalParams;
      type state = unit => unit;

      let name = "Service_Time.interval";
      let id = ({uniqueId, _}) => uniqueId;

      let init = (~params, ~dispatch) => {
        Revery.Tick.interval(
          ~name="Service_Time.interval: " ++ params.uniqueId,
          _ => {dispatch(Revery.Time.now())},
          params.every,
        );
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state) => {
        state();
      };
    });

  let interval = (~uniqueId: string, ~every, ~msg) => {
    TimerIntervalSubscription.create({every, uniqueId})
    |> Isolinear.Sub.map(current => msg(~current));
  };
};
