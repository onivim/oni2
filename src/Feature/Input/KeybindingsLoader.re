open Oni_Core;

module Log = (
  val Oni_Core.Log.withNamespace("Oni2.Feature.Input.KeybindingsLoader")
);

module File = {
  let loadKeybindings = (path: FpExp.t(FpExp.absolute)) => {
    let loadResult =
      path
      |> FpExp.toString
      |> Utility.JsonEx.from_file
      |> Utility.ResultEx.flatMap(Keybindings.of_yojson_with_errors);

    switch (loadResult) {
    | Ok((bindings, errors)) => (bindings, errors)
    | Error(msg) => ([], [msg])
    };
  };

  type params = {
    filePath: FpExp.t(FpExp.absolute),
    tick: int,
  };
  // TODO: Once we've fixed the issue with the Service_OS.FileWatcher,
  // this could be mostly replaced with it (and then there'd no longer
  // need to be an awkward 'notifyFileSaved' on the public interface)
  module Sub =
    Isolinear.Sub.Make({
      type nonrec msg = (list(Schema.resolvedKeybinding), list(string));
      type nonrec params = params;
      type state = unit;

      let name = "Feature_Input.KeybindingsLoader.FileSubscription";

      let id = ({filePath, tick}) =>
        FpExp.toString(filePath) ++ string_of_int(tick);

      let init = (~params, ~dispatch) => {
        Log.infof(m =>
          m(
            "Reloading keybindings file: %s",
            FpExp.toString(params.filePath),
          )
        );
        dispatch(loadKeybindings(params.filePath));
        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state as _) => ();
    });

  let sub = (~filePath, ~saveTick) => {
    Sub.create({filePath, tick: saveTick});
  };
};

type t =
  | None
  | File({
      filePath: FpExp.t(FpExp.absolute),
      saveTick: int,
    });

let none = None;

let file = filePath => File({filePath, saveTick: 0});

let notifyFileSaved = path =>
  fun
  | None => None
  | File({filePath, saveTick}) as orig =>
    if (FpExp.eq(filePath, path)) {
      File({filePath, saveTick: saveTick + 1});
    } else {
      orig;
    };

let sub =
  fun
  | None => Isolinear.Sub.none
  | File({saveTick, filePath}) => File.sub(~filePath, ~saveTick);
