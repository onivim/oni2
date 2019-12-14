/*
 * ProviderUtility.re
 */

module Core = Oni_Core;
module Utility = Core.Utility;
module Option = Utility.Option;

let runIfSelectorPasses = (~buffer, ~selector, f) => {
  Core.Buffer.getFileType(buffer)
  |> Option.map(DocumentSelector.matches(selector))
  |> Option.bind(matches =>
       if (matches) {
         Some(f());
       } else {
         None;
       }
     );
};
