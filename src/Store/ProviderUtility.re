/*
 * ProviderUtility.re
 */

open Oni_Core;

let runIfSelectorPasses = (~buffer, ~selector, f) => {
  Buffer.getFileType(buffer)
  |> Option.map(filetype =>
       Exthost.DocumentSelector.matches(~filetype, selector)
     )
  |> Utility.OptionEx.flatMap(matches =>
       if (matches) {
         Some(f());
       } else {
         None;
       }
     );
};
