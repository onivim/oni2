/*
 * LanguageFeaturesReducer.re
 */

open Oni_Model;
open Actions;

let reduce = (action: Actions.t, state: LanguageFeatures.t) => {
  switch (action) {
  | LanguageFeatureRegisterCompletionProvider(id, provider) =>
    LanguageFeatures.registerCompletionProvider(~id, ~provider, state)
  | _ => state
  };
};
