/* Feature_Hover.re
     This feature project contains logic related to Hover
   */
open Oni_Core;
open Revery;
open Revery.UI;
open EditorCoreTypes;

module Log = (val Log.withNamespace("Oni.Feature.Hover"));

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
};

type model = {
  shown: bool,
  providers: list(provider),
  contents: list(string),
  range: option(EditorCoreTypes.Range.t),
};

let initial = {shown: false, providers: [], contents: [], range: None};

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string)
  | ProviderRegistered(provider)
  | HoverInfoReceived({
      contents: list(string),
      range: option(EditorCoreTypes.Range.t),
    })
  | HoverRequestFailed(string);

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
               | Ok({contents, range}) =>
                 HoverInfoReceived({
                   contents,
                   range: Option.map(Exthost.OneBasedRange.toRange, range),
                 })
               | Error(s) => HoverRequestFailed(s)
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
  | HoverInfoReceived({contents, range}) => (
      {...model, contents, range},
      Nothing,
    )
  | _ => (model, Nothing)
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

module Styles = {
  open Style;

  let container = (~x, ~y) => [position(`Absolute), top(y), left(x)];
};

module View = {
  let make =
      (
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        ~model,
        ~editor: Feature_Editor.Editor.t,
        ~buffer,
        ~gutterWidth,
        ~cursorOffset,
        (),
      ) => {
    let grammars = Oni_Syntax.GrammarRepository.create(languageInfo);
    switch (model.range, model.shown) {
    | (Some(range), true) =>
      let y =
        int_of_float(
          editorFont.measuredHeight
          *. float(Index.toZeroBased(range.start.line) + 1)
          -. editor.scrollY
          +. 0.5,
        );

      let x =
        int_of_float(
          gutterWidth
          +. editorFont.measuredWidth
          *. float(Index.toZeroBased(range.start.column))
          -. editor.scrollX
          +. 0.5,
        );

      <View style={Styles.container(~x, ~y)}>
        {List.map(
           markdown =>
             <Oni_Components.Markdown
               colorTheme
               tokenTheme
               languageInfo
               fontFamily={uiFont.family}
               codeFontFamily={editorFont.fontFamily}
               grammars
               markdown
             />,
           model.contents,
         )
         |> React.listToElement}
      </View>;
    | (None, true) =>
      let cursorPosition =
        Feature_Editor.Editor.getPrimaryCursor(~buffer, editor);
      let y =
        int_of_float(
          editorFont.measuredHeight
          *. float(Index.toZeroBased(cursorPosition.line) + 1)
          -. editor.scrollY
          +. 0.5,
        );
      let x =
        int_of_float(
          gutterWidth
          +. editorFont.measuredWidth
          *. float(cursorOffset)
          -. editor.scrollX
          +. 0.5,
        );
      <View style={Styles.container(~x, ~y)}>
        {List.map(
           markdown =>
             <Oni_Components.Markdown
               colorTheme
               tokenTheme
               languageInfo
               fontFamily={uiFont.family}
               codeFontFamily={editorFont.fontFamily}
               grammars
               markdown
             />,
           model.contents,
         )
         |> React.listToElement}
      </View>;
    | _ => React.empty
    };
  };
};
