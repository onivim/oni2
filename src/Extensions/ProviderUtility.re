/*
 * ProviderUtility.re
 */

module Core = Oni_Core;

let runIfSelectorPasses = (~buffer, ~selector, f) => {
  Core.Buffer.getFileType(buffer)
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
