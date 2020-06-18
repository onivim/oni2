/* Markdown.re

    This adapts Revery's built in Markdown component to our code highlighters
   */

open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Syntax;

module Colors = Feature_Theme.Colors;

open {
       let syntaxHighlighter =
           (
             ~tokenTheme,
             ~colorTheme,
             ~languageInfo,
             ~grammars,
             ~language,
             ~defaultLanguage,
             lines,
           ) => {
         let grammarRepository =
           Textmate.GrammarRepository.create(scope =>
             GrammarRepository.getGrammar(~scope, grammars)
           );

         let scope =
           switch (
             Oni_Extensions.LanguageInfo.getScopeFromLanguage(
               languageInfo,
               language,
             )
           ) {
           | Some(scope) as s => s
           | None =>
             Oni_Extensions.LanguageInfo.getScopeFromLanguage(
               languageInfo,
               defaultLanguage,
             )
           };

         switch (scope) {
         | Some(scope) =>
           let tokenizerJob =
             TextmateTokenizerJob.create(
               ~scope,
               ~theme=tokenTheme,
               ~grammarRepository,
               Array.of_list(lines),
             )
             |> Job.tick(~budget=Some(0.1));

           List.init(
             List.length(lines),
             i => {
               let tokens =
                 TextmateTokenizerJob.getTokenColors(i, tokenizerJob);
               List.map(
                 (token: ThemeToken.t) => {
                   Markdown.SyntaxHighlight.makeHighlight(
                     ~byteIndex=token.index,
                     ~color=token.foregroundColor,
                     ~bold=token.bold,
                     ~italic=token.italic,
                   )
                 },
                 tokens,
               );
             },
           );
         | None =>
           // TODO: Replace this with SyntaxHighlight.default when revery#906 is merged
           List.init(List.length(lines), _ =>
             [
               Markdown.SyntaxHighlight.makeHighlight(
                 ~byteIndex=0,
                 ~color=Colors.Editor.foreground.from(colorTheme),
                 ~bold=false,
                 ~italic=false,
               ),
             ]
           )
         };
       };

       module Styles = {
         open Style;
         module Colors = Feature_Theme.Colors;

         let text = (~theme) => [color(Colors.foreground.from(theme))];

         let linkActive = (~theme) => [
           color(Colors.TextLink.activeForeground.from(theme)),
         ];
         let linkInactive = (~theme) => [
           color(Colors.TextLink.foreground.from(theme)),
         ];
       };
     };

let make =
    (
      ~colorTheme,
      ~tokenTheme,
      ~markdown,
      ~languageInfo,
      ~grammars,
      ~fontFamily,
      ~codeFontFamily,
      ~baseFontSize=16.,
      ~codeBlockStyle=?,
      ~defaultLanguage="",
      (),
    ) => {
  let textStyle = Styles.text(~theme=colorTheme);
  <Markdown
    syntaxHighlighter={syntaxHighlighter(
      ~tokenTheme,
      ~colorTheme,
      ~languageInfo,
      ~grammars,
      ~defaultLanguage,
    )}
    markdown
    fontFamily
    baseFontSize
    codeFontFamily
    paragraphStyle=textStyle
    h1Style=textStyle
    h2Style=textStyle
    h3Style=textStyle
    h4Style=textStyle
    h5Style=textStyle
    h6Style=textStyle
    inlineCodeStyle=textStyle
    activeLinkStyle={Styles.linkActive(~theme=colorTheme)}
    inactiveLinkStyle={Styles.linkInactive(~theme=colorTheme)}
    ?codeBlockStyle
  />;
};
