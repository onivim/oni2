/*
 * TitleStoreConnector
 *
 * This implements an updater (reducer + side effects) for the window title
 */
open Oni_Core;
open Oni_Model;
open Utility;

module Log = (val Log.withNamespace("Oni2.Store.Title"));

module Internal = {
  type titleClickBehavior =
    | Maximize
    | Minimize;

  let getTitleDoubleClickBehavior = () => {
    switch (Revery.Environment.os) {
    | Mac =>
      try({
        let ic =
          Unix.open_process_in(
            "defaults read 'Apple Global Domain' AppleActionOnDoubleClick",
          );
        let operation = input_line(ic);
        switch (operation) {
        | "Maximize" => Maximize
        | "Minimize" => Minimize
        | _ => Maximize
        };
      }) {
      | _exn =>
        Log.warn(
          "
          Unable to read default behavior for AppleActionOnDoubleClick",
        );
        Maximize;
      }
    | _ => Maximize
    };
  };
};

let start = (maximize, minimize, restore, close) => {
  let _lastTitle = ref("");

  let internalDoubleClickEffect =
    Isolinear.Effect.create(~name="maximize", () =>
      switch (Internal.getTitleDoubleClickBehavior()) {
      | Maximize => maximize()
      | Minimize => minimize()
      }
    );

  let internalWindowCloseEffect =
    Isolinear.Effect.create(~name="window.close", () => close());
  let internalWindowMaximizeEffect =
    Isolinear.Effect.create(~name="window.maximize", () => maximize());
  let internalWindowMinimizeEffect =
    Isolinear.Effect.create(~name="window.minimize", () => minimize());
  let internalWindowRestoreEffect =
    Isolinear.Effect.create(~name="window.restore", () => restore());

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | TitleBar(Feature_TitleBar.TitleDoubleClicked) => (
        state,
        internalDoubleClickEffect,
      )
    | TitleBar(Feature_TitleBar.WindowCloseClicked) => (
        state,
        internalWindowCloseEffect,
      )
    | TitleBar(Feature_TitleBar.WindowMaximizeClicked) => (
        state,
        internalWindowMaximizeEffect,
      )
    | TitleBar(Feature_TitleBar.WindowRestoreClicked) => (
        state,
        internalWindowRestoreEffect,
      )
    | TitleBar(Feature_TitleBar.WindowMinimizeClicked) => (
        state,
        internalWindowMinimizeEffect,
      )

    | _ => (state, Isolinear.Effect.none)
    };
  };
  updater;
};
