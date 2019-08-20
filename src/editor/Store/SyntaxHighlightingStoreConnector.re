/*
 * SyntaxHighlightingStoreConnector.re
 *
 * This connects syntax highlighting to the store, using various strategies:
 * - ReasonML parser
 * - Tree-Sitter
 * - TextMate grammars
 */

open Rench;

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

open Oni_Syntax.SyntaxHighlights;

module Log = Core.Log;

open Treesitter;

let start = (_languageInfo: Model.LanguageInfo.t, _setup: Core.Setup.t) => {

  let (stream, dispatch) = Isolinear.Stream.create();
        /*let (tree, _) = Treesitter.ArrayParser.parse(parser, None, Model.Buffer.getLines(buffer));
        let node = Treesitter.Tree.getRootNode(tree);
        print_endline(Treesitter.Node.toString(node));*/

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
//    | Model.Actions.Tick => (state, pumpEffect)
    | Model.Actions.BufferEnter(be) => {
    print_endline ("bufferenter: " ++ string_of_int(be.id));
    ({
      ...state,
      syntaxHighlighting2: Core.IntMap.add(be.id, SyntaxHighlights.empty, state.syntaxHighlighting2)
    }, Isolinear.Effect.none);
    }
    | _ => default
    };
  };

  (updater, stream);
};
