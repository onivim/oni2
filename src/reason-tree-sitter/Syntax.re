/*
     Syntax.re

    Helpers to connect TSNode with a syntax highlighting solution
 */

open EditorCoreTypes;

let getErrorRanges = (node: Node.t) => {
  let rec f = (node: Node.t, errors: list(Range.t)) => {
    let hasError = Node.hasError(node);

    switch (hasError) {
    | false => errors
    | true =>
      let isError = Node.isError(node);
      isError
        ? [Node.getRange(node), ...errors]
        : {
          let children = Node.getChildren(node);
          let i = ref(0);
          List.fold_left(
            (prev, curr) => {
              incr(i);
              f(curr, prev);
            },
            errors,
            children,
          );
        };
    };
  };

  f(node, []);
};

type scope = (int, string);

module Token = {
  type t = (Location.t, Location.t, list(scope), string);

  let ofNode = (~getTokenName, scopes, node: Node.t) => {
    let isNamed = Node.isNamed(node);
    let start = Node.getStartPoint(node);
    let stop = Node.getEndPoint(node);
    let range = Range.create(~start, ~stop);
    let tokenName = getTokenName(range);

    switch (isNamed) {
    | false => (start, stop, scopes, tokenName)
    | true =>
      let nodeType = Node.getType(node);
      let scopes = [(0, nodeType), ...scopes];
      (start, stop, scopes, tokenName);
    };
  };

  let getPosition = ((pos, _, _, _): t) => pos;

  let getEndPosition = ((_, endPos, _, _): t) => endPos;

  let getName = ((_, _, _, name): t) => name;

  let _showScope = ((idx, s)) => Printf.sprintf("(%n:%s)", idx, s);

  let show = ((p, e, scopes, tok): t) =>
    Printf.sprintf(
      "Token(%s - %s:%s|%s)",
      Location.toString(p),
      Location.toString(e),
      scopes |> List.map(_showScope) |> String.concat("."),
      tok,
    );
};

let getParentScopes = (node: Node.t) => {
  let rec f = (node: Node.t, scopes: list(scope)) => {
    let parent = Node.getParent(node);

    if (Node.isNull(parent)) {
      scopes;
    } else {
      Node.isNamed(parent)
        ? f(
            parent,
            [
              (Node.getBoundedNamedIndex(parent), Node.getType(parent)),
              ...scopes,
            ],
          )
        : f(parent, scopes);
    };
  };

  f(node, []);
};

let createArrayTokenNameResolver = (v: array(string), range: Range.t) =>
  if (range.start.line != range.stop.line) {
    "";
  } else if (range.start.line >= Index.fromZeroBased(Array.length(v))) {
    "";
  } else {
    let lineNumber = Index.toZeroBased(range.start.line);
    let line = v[lineNumber];

    let len = String.length(line);

    if (len == 0 || range.start.column == range.stop.column) {
      "";
    } else {
      Printf.sprintf(
        {|"%s"|},
        String.sub(
          line,
          Index.toZeroBased(range.start.column),
          Index.toZeroBased(range.stop.column)
          - Index.toZeroBased(range.start.column),
        ),
      );
    };
  };

let getTokens = (~getTokenName, ~range: Range.t, node: Node.t) => {
  let nodeToUse =
    Node.getDescendantForPointRange(
      node,
      Index.toZeroBased(range.start.line),
      Index.toZeroBased(range.start.column),
      Index.toZeroBased(range.stop.line),
      Index.toZeroBased(range.stop.column),
    );

  let parentScopes = getParentScopes(node) |> List.rev;

  let rec f = (index, n: Node.t, tokens: list(Token.t), scopes: list(scope)) => {
    let start = Node.getStartPoint(n);
    let stop = Node.getEndPoint(n);

    if (stop.line < range.start.line
        || stop.line == range.start.line
        && stop.column < range.start.column
        || start.line > range.stop.line
        || start.line == range.stop.line
        && stop.column > range.stop.column) {
      tokens;
    } else {
      let childCount = Node.getChildCount(n);

      switch (childCount) {
      | 0 => [Token.ofNode(~getTokenName, scopes, n), ...tokens]
      | _ =>
        let children = Node.getChildren(n);
        let newScopes =
          switch (Node.isNamed(n)) {
          | false => scopes
          | true => [(index, Node.getType(n)), ...scopes]
          };
        let (_, tokens) =
          List.fold_left(
            (prev, curr) => {
              let (index, tokens) = prev;
              let idx =
                Node.isNamed(curr) ? Node.getBoundedNamedIndex(curr) : index;
              let newTokens = f(idx, curr, tokens, newScopes);
              (idx, newTokens);
            },
            (0, tokens),
            children,
          );
        tokens;
      };
    };
  };

  f(0, nodeToUse, [], parentScopes) |> List.rev;
};
