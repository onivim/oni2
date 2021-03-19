open Kernel;

module Make = (()) => {
  let _bufferTracker: ref(IntMap.t(int)) = ref(IntMap.empty);

  let startTracking = bufferId => {
    _bufferTracker :=
      _bufferTracker^
      |> IntMap.update(
           bufferId,
           fun
           | None => Some(1)
           | Some(count) => Some(count + 1),
         );
  };

  let stopTracking = bufferId => {
    _bufferTracker :=
      _bufferTracker^
      |> IntMap.update(
           bufferId,
           fun
           | None => Some(0)
           | Some(count) => Some(count - 1),
         );
  };

  let isTracking = bufferId => {
    _bufferTracker^
    |> IntMap.find_opt(bufferId)
    |> Option.map(count => count >= 1)
    |> Option.value(~default=false);
  };
};
