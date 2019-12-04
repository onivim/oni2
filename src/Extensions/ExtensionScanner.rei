/*
 * ExtensionScanner.re
 *
 * Module to get and discover extension manifests
 */

type t = {
  manifest: ExtensionManifest.t,
  path: string,
};

let scan: (~prefix: option(string)=?, string) => list(t);
