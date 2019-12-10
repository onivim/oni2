/*
 * LanguageFeaturesReducer.re
 */

open Oni_Model;
open Actions;

let reduce = (action: Actions.t, state: LanguageFeatures.t) => {
  switch (action) {
  | LanguageFeatureRegisterCompletionProvider(sp) =>
    LanguageFeatures.registerCompletionProvider(sp, state)
  | _ => state
  };
};
