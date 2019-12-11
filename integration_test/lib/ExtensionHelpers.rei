/*
 * LanguageHelpers.rei
 *
 * Helper utility for language features
 */

open Types;

let waitForExtensionToActivate: (~extensionId: string, waitForState) => unit;

/* [waitForNewSuggestProviders(f)] waits for suggest providers to be registered,
       after trigger the action [f].
   */
let waitForNewCompletionProviders:
  (~description: string, unit => unit, waitForState) => unit;
