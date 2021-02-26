// SubEx.re - additional utilities for Isolinear.Sub

type unitParams = {uniqueId: string};
module UnitSubscription =
  Isolinear.Sub.Make({
    type nonrec msg = unit;

    type nonrec params = unitParams;

    type state = unit;

    let name = "Oni_Core.SubEx.UnitSubscription";
    let id = params => params.uniqueId;

    let init = (~params as _, ~dispatch) => {
      dispatch();
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state as _) => {
      ();
    };
  });

let unit = (~uniqueId) => UnitSubscription.create({uniqueId: uniqueId});

// TODO: This should be in `Isolinear.Sub`
let value = (~uniqueId, v) => unit(~uniqueId) |> Isolinear.Sub.map(() => v);
