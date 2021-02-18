module Log = (
  val Kernel.Log.withNamespace("Core.SubEx")
);

module JsonFile = {
  let loadJson = (path: Fp.t(Fp.absolute)) => {
      path
      |> Fp.toString
      |> Utility.JsonEx.from_file;
  };

  type params = {
    uniqueId: string,
    filePath: Fp.t(Fp.absolute),
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
        uniqueId ++ Fp.toString(filePath) ++ string_of_int(tick);

      let init = (~params, ~dispatch) => {
        Log.infof(m =>
          m("Reloading json file (%s): %s", params.uniqueId, Fp.toString(params.filePath))
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

let jsonFile = (~uniqueId, ~filePath, ~tick) => JsonFile.Sub.create({
  uniqueId,
  filePath,
  tick
});
