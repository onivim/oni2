/* Feature_Hover.re
     This feature project contains logic related to Hover
   */

[@deriving show({with_path: false})]
type command =
  | Show;

module Commands = {
  open Feature_Commands.Schema;

  let show =
    define(
      ~category="Hover",
      ~title="Show hover panel",
      "editor.action.showHover",
      Command(Show),
    );
};
