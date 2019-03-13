/*
 * ExtensionScanner.re
 *
 * Module to get and discover extension manifests
 */

type t = {
  manifest: ExtensionManifest.t,
  path: string,
};

let scan: string => list(t);

let getGrammars: list(t) => list(ExtensionContributions.Grammar.t);
