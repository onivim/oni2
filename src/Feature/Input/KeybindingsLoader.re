open Oni_Core;

module Log = (
  val Oni_Core.Log.withNamespace("Oni2.Feature.Input.KeybindingsLoader")
);

module File = {
  let loadKeybindings = (loadResult: result(Yojson.Safe.t, string)) => {
    let parseResult =
      loadResult
      |> Utility.ResultEx.flatMap(Keybindings.of_yojson_with_errors);

    switch (parseResult) {
    | Ok((bindings, errors)) => (bindings, errors)
    | Error(msg) => ([], [msg])
    };
  };

  let sub = (~filePath, ~saveTick) => {
    Oni_Core.SubEx.jsonFile(
      ~uniqueId="Feature_Input.KeybindingsLoader",
      ~filePath,
      ~tick=saveTick,
    )
    |> Isolinear.Sub.map(loadKeybindings);
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

let getFilePath: t => option(FpExp.t(FpExp.absolute)) =
  fun
  | None => None
  | File({filePath, _}) => Some(filePath);

let sub =
  fun
  | None => Isolinear.Sub.none
  | File({saveTick, filePath}) => File.sub(~filePath, ~saveTick);
