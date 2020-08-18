/*
 * ProviderUtility.re
 */

open Oni_Core;

let runIfSelectorPasses = (~buffer, ~selector, f) => {
  let filetype = buffer |> Buffer.getFileType |> Buffer.FileType.toString;

  if (Exthost.DocumentSelector.matches(~filetype, selector)) {
    Some(f());
  } else {
    None;
  };
};
