/* Feature_Hover.re
     This feature project contains logic related to Hover
   */
open Oni_Core;

type model = {shown: bool};

let initial = {shown: false};

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string);

let update = (model, msg) =>
  switch (msg) {
  | Command(Show) => {shown: true}
  | KeyPressed(_) => {shown: false}
  };

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

module Contributions = {
  let commands = Commands.[show];
};

module View = {
  open Revery;
  open Revery.UI;

  let make =
      (
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~fontFamily,
        ~codeFontFamily,
        ~model,
        (),
      ) => {
    let grammars = Oni_Syntax.GrammarRepository.create(languageInfo);
    model.shown
      ? <View>
          <Oni_Components.Markdown
            colorTheme
            tokenTheme
            languageInfo
            fontFamily
            codeFontFamily
            grammars
            markdown="
```reason
let x = 4;
```
```reasonml
let x = 4;
```
            "
          />
        </View>
      : React.empty;
  };
};
