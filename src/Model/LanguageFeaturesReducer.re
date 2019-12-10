/*
 * LanguageFeaturesReducer.re
 */

open Oni_Extensions;

open Actions;

let reduce = (action: Actions.t, state: LanguageFeatures.t) => {
  switch (action) {
  | LanguageFeatureRegisterDefinitionProvider(dp) =>
    LanguageFeatures.registerDefinitionProvider(dp, state)
  | LanguageFeatureRegisterSuggestProvider(sp) =>
    LanguageFeatures.registerSuggestProvider(sp, state)
  | _ => state
  };
};
