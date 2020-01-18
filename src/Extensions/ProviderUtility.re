/*
 * ProviderUtility.re
 */

module Core = Oni_Core;
module Utility = Core.Utility;
module Option = Utility.Option;
module OptionEx = Utility.OptionEx;

let runIfSelectorPasses = (~buffer, ~selector, f) => {
  Core.Buffer.getFileType(buffer)
  |> Option.map(DocumentSelector.matches(selector))
  |> OptionEx.flatMap(matches =>
       if (matches) {
         Some(f());
       } else {
         None;
       }
     );
};
