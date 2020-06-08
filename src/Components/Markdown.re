/* Markdown.re

    This adapts Revery's built in Markdown component to our code highlighters
   */

open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Syntax;

open {
       let syntaxHighlighter =
           (~theme, ~languageInfo, ~grammars, ~language, lines) => {
         let grammarRepository =
           Textmate.GrammarRepository.create(scope =>
             GrammarRepository.getGrammar(~scope, grammars)
           );

         let scope =
           Oni_Extensions.LanguageInfo.getScopeFromLanguage(
             languageInfo,
             language,
           )
           |> Option.value(~default="source.plaintext");

         let tokenizerJob =
           TextmateTokenizerJob.create(
             ~scope,
             ~theme,
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
               (token: ColorizedToken.t) =>
                 Markdown.SyntaxHighlight.makeHighlight(
                   ~byteIndex=token.index,
                   ~color=token.foregroundColor,
                   ~bold=false,
                   ~italicized=false,
                 ),
               tokens,
             );
           },
         );
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
      (),
    ) => {
  let textStyle = Styles.text(~theme=colorTheme);
  <Markdown
    syntaxHighlighter={syntaxHighlighter(
      ~theme=tokenTheme,
      ~languageInfo,
      ~grammars,
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
  />;
};
