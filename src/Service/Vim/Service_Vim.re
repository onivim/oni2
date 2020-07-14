open Oni_Core;
module Log = (val Log.withNamespace("Service_Vim"));

let forceReload = () =>
  Isolinear.Effect.create(~name="vim.discardChanges", () =>
    ignore(Vim.command("e!"): Vim.Context.t)
  );

let forceOverwrite = () =>
  Isolinear.Effect.create(~name="vim.forceOverwrite", () =>
    ignore(Vim.command("w!"): Vim.Context.t)
  );

let reload = () =>
  Isolinear.Effect.create(~name="vim.reload", () => {
    ignore(Vim.command("e"): Vim.Context.t)
  });

let saveAllAndQuit = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    ignore(Vim.command("xa"): Vim.Context.t)
  );

let quitAll = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    ignore(Vim.command("qa!"): Vim.Context.t)
  );

module Effects = {
  let paste = (~toMsg, text) => {
    Isolinear.Effect.createWithDispatch(~name="vim.clipboardPaste", dispatch => {
      let isCmdLineMode = Vim.Mode.getCurrent() == Vim.Types.CommandLine;
      let isInsertMode = Vim.Mode.getCurrent() == Vim.Types.Insert;

      if (isInsertMode || isCmdLineMode) {
        if (!isCmdLineMode) {
          Vim.command("set paste") |> ignore;
        };

        Log.infof(m => m("Pasting: %s", text));
        let latestContext: Vim.Context.t = Oni_Core.VimEx.inputString(text);

        if (!isCmdLineMode) {
          Vim.command("set nopaste") |> ignore;
          dispatch(toMsg(latestContext.cursors));
        };
      };
    });
  };

  let getRegisterValue = (~toMsg, char) =>
    Isolinear.Effect.createWithDispatch(~name="vim.getRegisterValue", dispatch => {
      let result = Vim.Registers.get(~register=char);
      dispatch(toMsg(result));
    });

  let applyEdits =
      (
        ~bufferId: int,
        ~version: int,
        ~edits: list(Vim.Edit.t),
        toMsg: result(unit, string) => 'msg,
      ) => {
    Isolinear.Effect.createWithDispatch(~name="vim.applyEdits", dispatch => {
      Log.infof(m => m("Trying to apply edits to buffer: %d", bufferId));
      let maybeBuffer = Vim.Buffer.getById(bufferId);
      let result =
        switch (maybeBuffer) {
        | None =>
          Error("No buffer found with id: " ++ string_of_int(bufferId))
        | Some(buffer) =>
          let bufferVersion = Vim.Buffer.getVersion(buffer);
          if (bufferVersion < version) {
            Error(
              Printf.sprintf(
                "Expected buffer version %d, got %d",
                bufferVersion,
                version,
              ),
            );
          } else {
            Vim.Buffer.applyEdits(~edits, buffer);
          };
        };

      result |> toMsg |> dispatch;
    });
  };
};

module Sub = {
  module EvalSubscription =
    Isolinear.Sub.Make({
      type state = unit;
      type params = string;
      type msg = result(string, string);
      let name = "Vim.Sub.Eval";
      let id = params => params;
      let init = (~params, ~dispatch) => {
        Vim.eval(params) |> dispatch;
      };
      let update = (~params as _, ~state as _, ~dispatch as _) => {
        ();
      };
      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });
  let eval = (~toMsg, expression) => {
    EvalSubscription.create(expression) |> Isolinear.Sub.map(toMsg);
  };
};
