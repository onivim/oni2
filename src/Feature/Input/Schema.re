open Oni_Core;
open Utility;
module Log = (val Log.withNamespace("Oni2.Feature.Input"));

[@deriving show]
type keybinding =
  | Binding({
      key: string,
      command: string,
      arguments: Yojson.Safe.t,
      condition: WhenExpr.t,
    })
  | Remap({
      allowRecursive: bool,
      fromKeys: string,
      toKeys: string,
      condition: WhenExpr.t,
    });

type resolvedKeybinding =
  | ResolvedBinding({
      matcher: EditorInput.Matcher.t,
      command: InputStateMachine.execute,
      condition: WhenExpr.ContextKeys.t => bool,
      rawCondition: WhenExpr.t,
    })
  | ResolvedRemap({
      allowRecursive: bool,
      matcher: EditorInput.Matcher.t,
      toKeys: list(EditorInput.KeyPress.t),
      condition: WhenExpr.ContextKeys.t => bool,
      rawCondition: WhenExpr.t,
    });

let resolvedToString =
  fun
  | ResolvedBinding({matcher, command, rawCondition, _}) => {
      Printf.sprintf(
        "Binding - command: %s matcher: %s when: %s",
        InputStateMachine.executeToString(command),
        EditorInput.Matcher.toString(matcher),
        WhenExpr.show(rawCondition),
      );
    }
  | ResolvedRemap({allowRecursive, matcher, toKeys, rawCondition, _}) => {
      Printf.sprintf(
        "Remap - rec: %b, matcher: %s to: %s when: %s",
        allowRecursive,
        EditorInput.Matcher.toString(matcher),
        toKeys
        |> List.map(EditorInput.KeyPress.toString)
        |> String.concat(","),
        WhenExpr.show(rawCondition),
      );
    };

let bind = (~key, ~command, ~condition) =>
  Binding({key, command, arguments: `Null, condition});

let bindWithArgs = (~arguments, ~key, ~command, ~condition) =>
  Binding({key, command, arguments, condition});

let mapCommand = (~f, keybinding: keybinding) => {
  switch (keybinding) {
  | Binding(binding) => Binding({...binding, command: f(binding.command)})
  | Remap(_) as remap => remap
  };
};

let clear = (~key as _) => failwith("Not implemented");

let remap = (~allowRecursive, ~fromKeys, ~toKeys, ~condition) =>
  Remap({allowRecursive, fromKeys, toKeys, condition});

let resolve = keybinding => {
  let evaluateCondition = (whenExpr, contextKeys) => {
    WhenExpr.evaluate(whenExpr, WhenExpr.ContextKeys.getValue(contextKeys));
  };

  switch (keybinding) {
  | Binding({key, command, arguments, condition}) =>
    let maybeMatcher =
      EditorInput.Matcher.parse(~explicitShiftKeyNeeded=true, key);
    maybeMatcher
    |> Stdlib.Result.map(matcher => {
         ResolvedBinding({
           matcher,
           command: InputStateMachine.NamedCommand({command, arguments}),
           condition: evaluateCondition(condition),
           rawCondition: condition,
         })
       });

  | Remap({allowRecursive, fromKeys, condition, toKeys}) =>
    let evaluateCondition = (whenExpr, contextKeys) => {
      WhenExpr.evaluate(
        whenExpr,
        WhenExpr.ContextKeys.getValue(contextKeys),
      );
    };

    let maybeMatcher =
      EditorInput.Matcher.parse(~explicitShiftKeyNeeded=true, fromKeys);

    let maybeKeys =
      EditorInput.KeyPress.parse(~explicitShiftKeyNeeded=true, toKeys);

    ResultEx.map2(
      (matcher, toKeys) => {
        ResolvedRemap({
          allowRecursive,
          matcher,
          condition: evaluateCondition(condition),
          toKeys,
          rawCondition: condition,
        })
      },
      maybeMatcher,
      maybeKeys,
    );
  };
};
