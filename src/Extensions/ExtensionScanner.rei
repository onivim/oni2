/*
 * ExtensionScanner.re
 *
 * Module to get and discover extension manifests
 */

type category =
  | Default
  | Bundled
  | User
  | Development;

type t = {
  category,
  manifest: ExtensionManifest.t,
  path: string,
};

let scan:
  (~prefix: option(string)=?, ~category: category, string) => list(t);
