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

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update = (~maybeBuffer, ~maybeEditor, ~extHostClient, model, msg) =>
  switch (msg) {
  | Command(Show) =>
    switch (maybeBuffer, maybeEditor) {
    | (Some(buffer), Some(editor)) =>
      let filetype =
        buffer
        |> Oni_Core.Buffer.getFileType
        |> Option.value(~default="plaintext");

      let matchingProviders =
        model.providers
        |> List.filter(({selector, _}) =>
             Exthost.DocumentSelector.matches(~filetype, selector)
           );

      let position = Feature_Editor.Editor.getPrimaryCursor(~buffer, editor);

      let effects =
        matchingProviders
        |> List.map(provider =>
             Service_Exthost.Effects.LanguageFeatures.provideHover(
               ~handle=provider.handle,
               ~uri=Oni_Core.Buffer.getUri(buffer),
               ~position,
               extHostClient,
               res =>
               switch (res) {
               | Ok(hover) =>
                 List.iter(h => print_endline(h), hover.contents);
                 failwith(
                   "Got data from handle: " ++ string_of_int(provider.handle),
                 );
               | Error(s) => failwith(s)
               }
             )
           )
        |> Isolinear.Effect.batch;

      ({...model, shown: true}, Effect(effects));

    | _ => (model, Nothing)
    }
  | KeyPressed(_) => ({...model, shown: false}, Nothing)
  | ProviderRegistered(provider) => (
      {...model, providers: [provider, ...model.providers]},
      Nothing,
    )
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
