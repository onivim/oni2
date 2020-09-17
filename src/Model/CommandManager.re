/*
 * CommandManager.re
 *
 * Top-level state of the editor
 */

open Oni_Core;

let current = state => {
  let focus = FocusManager.current(state);
  Command.Lookup.unionMany([
    Feature_Commands.all(state.commands),
    Feature_Extensions.commands(state.extensions)
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.Extensions(msg)),
    Feature_Search.Contributions.commands(
      ~isFocused={
        focus == Focus.Search;
      },
    )
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.Search(msg)),
    Feature_SCM.Contributions.commands(
      ~isFocused={
        focus == Focus.SCM;
      },
    )
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.SCM(msg)),
  ]);
};
