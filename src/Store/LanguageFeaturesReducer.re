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
    | DocumentSymbolProviderAvailable(id, provider) =>
      LanguageFeatures.registerDocumentSymbolProvider(~id, ~provider, state)
    | FindAllReferencesProviderAvailable(id, provider) =>
      LanguageFeatures.registerFindAllReferencesProvider(
        ~id,
        ~provider,
        state,
      )
    }
  | _ => state
  };
};
