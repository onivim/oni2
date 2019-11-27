/*
 * LanguageFeaturesReducer.re
 */

open Oni_Extensions;

open Actions;

let reduce = (action: Actions.t, state: LanguageFeatures.t) => {

  switch (action) {
  | LanguageFeatureRegisterSuggestProvider(sp) =>
    LanguageFeatures.registerSuggestProvider(sp, state);
  | _ => state
  };
};
