/* Feature_Hover.re
     This feature project contains logic related to Hover
   */

[@deriving show({with_path: false})]
type command =
  | Show
  | Hide;

module Commands = {
  open Feature_Commands.Schema;

  let show =
    define(
      ~category="Hover",
      ~title="Show hover panel",
      "hover.show",
      Command(Show),
    );

  let hide =
    define(
      ~category="Hover",
      ~title="Hide hover panel",
      "hover.hide",
      Command(Hide),
    );
};
