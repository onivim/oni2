/*
 * LanguageFeaturesReducer.re
 */

open Oni_Model;
open Actions;

open LanguageFeatures;

let reduce = (action: Actions.t, state: LanguageFeatures.t) => {
  switch (action) {
  | LanguageFeature(languageAction) =>
    switch (languageAction) {
    | CompletionProviderAvailable(id, provider) =>
      LanguageFeatures.registerCompletionProvider(~id, ~provider, state)
    | DefinitionProviderAvailable(id, provider) =>
      LanguageFeatures.registerDefinitionProvider(~id, ~provider, state)
    }
  | _ => state
  };
};
