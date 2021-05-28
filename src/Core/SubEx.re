module Log = (val Kernel.Log.withNamespace("Core.SubEx"));

module JsonFile = {
  let loadJson = (path: FpExp.t(FpExp.absolute)) => {
    path |> FpExp.toString |> Utility.JsonEx.from_file;
  };

  type params = {
    uniqueId: string,
    filePath: FpExp.t(FpExp.absolute),
    tick: int,
  };
  // TODO: Once we've fixed the issue with the Service_OS.FileWatcher,
  // this could be mostly replaced with it (and then there'd no longer
  // need to be an awkward 'notifyFileSaved' on the public interface)
  module Sub =
    Isolinear.Sub.Make({
      type nonrec msg = result(Yojson.Safe.t, string);
      type nonrec params = params;
      type state = unit;

      let name = "SubEx.FileSubscription";

      let id = ({uniqueId, filePath, tick}) =>
        uniqueId ++ FpExp.toString(filePath) ++ string_of_int(tick);

      let init = (~params, ~dispatch) => {
        Log.infof(m =>
          m(
            "Reloading json file (%s): %s",
            params.uniqueId,
            FpExp.toString(params.filePath),
          )
        );
        dispatch(loadJson(params.filePath));
        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state as _) => ();
    });
};

let jsonFile = (~uniqueId, ~filePath, ~tick) =>
  JsonFile.Sub.create({uniqueId, filePath, tick});

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

type taskParams = {
  name: string,
  uniqueId: string,
  task: unit => unit,
};
module TaskSubscription =
  Isolinear.Sub.Make({
    type nonrec msg = unit;

    type nonrec params = taskParams;

    type state = unit;

    let name = "Oni_Core.SubEx.TaskSubscription";
    let id = params => params.name ++ "." ++ params.uniqueId;

    let init = (~params, ~dispatch) => {
      params.task();
      dispatch();
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state as _) => {
      ();
    };
  });

let task = (~name, ~uniqueId, ~task) =>
  TaskSubscription.create({name, uniqueId, task});

// TODO: This should be in `Isolinear.Sub`
let value = (~uniqueId, v) => unit(~uniqueId) |> Isolinear.Sub.map(() => v);
