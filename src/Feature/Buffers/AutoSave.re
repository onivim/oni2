open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Model.Buffers.AutoSave"));
module Mode = {
  type t =
    | Off
    | AfterDelay
    | OnFocusChange
    | OnWindowChange;

  let default = Off;

  let decode =
    Json.Decode.(
      {
        string
        |> map(String.lowercase_ascii)
        |> and_then(
             fun
             | "afterdelay" => succeed(AfterDelay)
             | "onfocuschange" => succeed(OnFocusChange)
             | "onwindowchange" => succeed(OnWindowChange)
             | _ => succeed(Off),
           );
      }
    );

  let encode =
    Json.Encode.(
      {
        mode => {
          switch (mode) {
          | Off => string("Off")
          | AfterDelay => string("AfterDelay")
          | OnFocusChange => string("OnFocusChange")
          | OnWindowChange => string("OnWindowChange")
          };
        };
      }
    );
};

module Configuration = {
  open Config.Schema;

  let autoSave =
    setting(
      "files.autoSave",
      custom(~encode=Mode.encode, ~decode=Mode.decode),
      ~default=Off,
    );

  let autoSaveDelay =
    setting(
      "files.autoSaveDelay",
      time,
      ~default=Revery.Time.ofFloatSeconds(1.0),
    );
};

type model = {
  mode: Mode.t,
  delay: Revery.Time.t,
};

let initial = {mode: Mode.default, delay: Revery.Time.ofFloatSeconds(1.0)};

[@deriving show]
type msg =
  | Noop
  | AutoSaveTimerExpired
  | AutoSaveWindowLostFocus
  | AutoSaveBufferChangedFocus;

type outmsg =
  | Nothing
  | DoAutoSave;

let configurationChanged = (~config, _model) => {
  mode: Configuration.autoSave.get(config),
  delay: Configuration.autoSaveDelay.get(config),
};

let update = (msg, model) => {
  switch (msg) {
  | Noop => (model, Nothing)
  | AutoSaveWindowLostFocus
  | AutoSaveBufferChangedFocus
  | AutoSaveTimerExpired => (model, DoAutoSave)
  };
};

let sub = (~isWindowFocused, ~maybeFocusedBuffer, ~buffers, model) => {
  let anyDirty = buffers |> List.exists(buf => Buffer.isModified(buf));

  switch (model.mode) {
  | Mode.Off => Isolinear.Sub.none
  | Mode.AfterDelay =>
    if (anyDirty) {
      Service_Time.Sub.interval(
        ~uniqueId="Feature_Buffer.autoSave.delay",
        ~every=model.delay,
        ~msg=(~current as _) =>
        AutoSaveTimerExpired
      );
    } else {
      Isolinear.Sub.none;
    }
  | OnFocusChange =>
    let uniqueId =
      switch (maybeFocusedBuffer) {
      | None => "Feature_Buffer.autoSave.focusChange:None"
      | Some(id) => "Feature_Buffer.autoSave.focusChange" ++ string_of_int(id)
      };
    SubEx.value(~uniqueId, AutoSaveBufferChangedFocus);
  | OnWindowChange =>
    if (!isWindowFocused) {
      SubEx.value(
        ~uniqueId="Feature_Buffer.autoSave.windowNotFocused",
        AutoSaveWindowLostFocus,
      );
    } else {
      Isolinear.Sub.none;
    }
  };
};

module Contributions = {
  let configuration = Configuration.[autoSave.spec, autoSaveDelay.spec];
};
