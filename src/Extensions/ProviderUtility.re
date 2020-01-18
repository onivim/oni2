/*
 * ProviderUtility.re
 */

module Core = Oni_Core;
module Utility = Core.Utility;

let runIfSelectorPasses = (~buffer, ~selector, f) => {
  Core.Buffer.getFileType(buffer)
  |> Utility.Option.map(DocumentSelector.matches(selector))
  |> Utility.OptionEx.flatMap(matches =>
       if (matches) {
         Some(f());
       } else {
         None;
       }
     );
};
