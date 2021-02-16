// type t = {
//     //maybeStaticKeybindings: option(list(Schema.keybinding))
// };

module File = {
  let loadKeybindings = (path: Fp.t(Fp.absolute)) => {
    path
    |> Fp.toString
    |> Utility.JsonEx.from_file
    |> Utility.ResultEx.flatMap(Keybindings.of_yojson_with_errors)
    // Handle error case when parsing entire JSON file
    |> Result.value(~default=([], []));
  };

  type params = {
    filePath: Fp.t(Fp.absolute),
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
        Fp.toString(filePath) ++ string_of_int(tick);

      let init = (~params, ~dispatch) => {
        prerr_endline("Reloading: " ++ Fp.toString(params.filePath));
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
      filePath: Fp.t(Fp.absolute),
      saveTick: int,
    });

let none = None;

let file = filePath => File({filePath, saveTick: 0});

let notifyFileSaved = path =>
  fun
  | None => None
  | File({filePath, saveTick}) as orig =>
    if (Fp.eq(filePath, path)) {
      prerr_endline("Bump tick!");
      File({filePath, saveTick: saveTick + 1});
    } else {
      orig;
    };

let sub =
  fun
  | None => Isolinear.Sub.none
  | File({saveTick, filePath}) => File.sub(~filePath, ~saveTick);
