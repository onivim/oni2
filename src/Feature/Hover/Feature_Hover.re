/* Feature_Hover.re
     This feature project contains logic related to Hover
   */
open Oni_Core;
open Revery;
open Revery.UI;

module Log = (val Log.withNamespace("Oni.Feature.Hover"));

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
};

type model = {
  shown: bool,
  providers: list(provider),
};

let initial = {shown: false, providers: []};

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string)
  | ProviderRegistered(provider);

let update = (model, msg) =>
  switch (msg) {
  | Command(Show) => {...model, shown: true}
  | KeyPressed(_) => {...model, shown: false}
  | ProviderRegistered(provider) => {
      ...model,
      providers: [provider, ...model.providers],
    }
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
