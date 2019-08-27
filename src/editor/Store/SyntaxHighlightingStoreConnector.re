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
open Oni_Syntax.TreeSitterScopes;

module Log = Core.Log;

let start = (languageInfo: Model.LanguageInfo.t, setup: Core.Setup.t) => {
  let (stream, _dispatch) = Isolinear.Stream.create();
  /*let (tree, _) = Treesitter.ArrayParser.parse(parser, None, Model.Buffer.getLines(buffer));
    let node = Treesitter.Tree.getRootNode(tree);
    print_endline(Treesitter.Node.toString(node));*/
  
  let jsonTreeSitterScopes =
    setup.bundledExtensionsPath ++ "/json/syntaxes/tree-sitter-json.json";

  let parsedTreeSitterScopes = Yojson.Safe.from_file(jsonTreeSitterScopes);
  let treeSitterJson = Yojson.Safe.Util.member("scopes", parsedTreeSitterScopes);
  let treeSitterScopes = TextMateConverter.of_yojson(treeSitterJson);

  let getTreeSitterScopeMapper = () => {
    treeSitterScopes;
  };

  let getLines = (state: Model.State.t, id: int) => {
    switch (Model.Buffers.getBuffer(id, state.buffers)) {
    | None => [||]
    | Some(v) => Model.Buffer.getLines(v);
    }
  };

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Model.Actions.BufferUpdate(bu) =>
      print_endline ("Buffer update: " ++ Core.Types.BufferUpdate.show(bu));

      let lines = getLines(state, bu.id);
        let state = {
          ...state,
          syntaxHighlighting2:
            Core.IntMap.add(
              bu.id,
              SyntaxHighlights.create(
              ~theme=state.tokenTheme,
              ~getTreeSitterScopeMapper,
              lines),
              state.syntaxHighlighting2,
            ),
        };
      (state, Isolinear.Effect.none)
    | _ => default
    };
  };

  (updater, stream);
};
