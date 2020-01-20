/*
 * AutoClosingPairsConnector.re
 *
 * This module synchronizes auto-closing pairs from the Onivim 2
 * configuration to the Vim implementation.
 */

module Core = Oni_Core;
open Oni_Core_Kernel;
module Ext = Oni_Extensions;
module Model = Oni_Model;

module Log = (val Oni_Core_Kernel.Log.withNamespace("Oni2.Store.AutoClosingPairs"));

let start = (languageInfo: Ext.LanguageInfo.t) => {
  ignore(languageInfo);

  let autoClosingPairsEnabled = ref(false);

  let syncAutoClosingPairsEffect = (configuration: Core.Configuration.t) =>
    Isolinear.Effect.create(~name="apc.sync", () => {
      let acp =
        Core.Configuration.getValue(
          c => c.experimentalAutoClosingPairs,
          configuration,
        );
      if (autoClosingPairsEnabled^ != acp) {
        autoClosingPairsEnabled := acp;
        Log.debugf(m => m("AutoClosingPairs: Setting to %b", acp));

        switch (acp) {
        | false => Vim.Options.setAutoClosingPairs(false)
        | true =>
          Vim.Options.setAutoClosingPairs(true);
          Vim.AutoClosingPairs.create(
            Vim.AutoClosingPairs.[
              AutoClosingPair.create(~opening="`", ~closing="`", ()),
              AutoClosingPair.create(~opening={|"|}, ~closing={|"|}, ()),
              AutoClosingPair.create(~opening="[", ~closing="]", ()),
              AutoClosingPair.create(~opening="(", ~closing=")", ()),
              AutoClosingPair.create(~opening="{", ~closing="}", ()),
            ],
          )
          |> Vim.AutoClosingPairs.setPairs;
        };
      };
    });

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.ConfigurationSet(configuration) => (
        state,
        syncAutoClosingPairsEffect(configuration),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
