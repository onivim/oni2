[@deriving show]
type severity =
  | Ignore
  | Info
  | Warning
  | Error;

let intToSeverity =
  fun
  | 0 => Ignore
  | 1 => Info
  | 2 => Warning
  | 3 => Error
  | _ => Ignore;

[@deriving show]
type msg =
  | ShowMessage({
      severity,
      message: string,
      extensionId: option(string),
    });

let handle = (method, args: Yojson.Safe.t) => {
  switch (method, args) {
  | (
      "$showMessage",
      `List([`Int(severity), `String(message), options, ..._]),
    ) =>
    let extensionId =
      Yojson.Safe.Util.(
        options
        |> member("extension")
        |> member("identifier")
        |> to_string_option
      );
    Ok(
      ShowMessage({severity: intToSeverity(severity), message, extensionId}),
    );
  | _ =>
    Error(
      "Unable to parse method: "
      ++ method
      ++ " with args: "
      ++ Yojson.Safe.to_string(args),
    )
  };
};
