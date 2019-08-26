/*
 * SyntaxHighlightingStoreConnector.re
 *
 * This connects syntax highlighting to the store, using various strategies:
 * - ReasonML parser
 * - Tree-Sitter
 * - TextMate grammars
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

open Oni_Syntax.SyntaxHighlights;

module Log = Core.Log;

let start = (languageInfo: Model.LanguageInfo.t, _setup: Core.Setup.t) => {
  let (stream, _dispatch) = Isolinear.Stream.create();
  /*let (tree, _) = Treesitter.ArrayParser.parse(parser, None, Model.Buffer.getLines(buffer));
    let node = Treesitter.Tree.getRootNode(tree);
    print_endline(Treesitter.Node.toString(node));*/

  let getLines = (state: Model.State.t, id: int) => {
    switch (Model.Buffers.getBuffer(id, state.buffers)) {
    | None => [||]
    | Some(v) => Model.Buffer.getLines(v);
    }
  };

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    //    | Model.Actions.Tick => (state, pumpEffect)
    | Model.Actions.BufferUpdate(bu) =>
      print_endline ("Buffer update: " ++ string_of_int(bu.id));
        let lines = getLines(state, be.id);
        {
          ...state,
          syntaxHighlighting2:
            Core.IntMap.add(
              be.id,
              SyntaxHighlights.create(Treesitter, lines),
              state.syntaxHighlighting2,
            ),
        },
      (state, Isolinear.Effect.none)
    | _ => default
    };
  };

  (updater, stream);
};
