/*
 * CommandManager.re
 *
 * Top-level state of the editor
 */

open Oni_Core;

let current = state => {
  let focus = FocusManager.current(state);
  Command.Lookup.unionMany([
    Feature_Buffers.Contributions.commands
    |> Command.Lookup.map(msg => Actions.Buffers(msg)),
    Feature_Commands.all(state.commands),
    Feature_Extensions.Contributions.commands(
      ~isFocused={
        focus == Focus.Extensions;
      },
      state.extensions,
    )
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.Extensions(msg)),
    Feature_Search.Contributions.commands(
      ~isFocused={
        focus == Focus.Search;
      },
    )
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.Search(msg)),
    Feature_Explorer.Contributions.commands(
      ~isFocused={
        focus == Focus.FileExplorer;
      },
      state.fileExplorer,
    )
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.FileExplorer(msg)),
    Feature_Pane.Contributions.commands(
      ~isFocused={
        focus == Focus.Pane;
      },
      state.pane,
    )
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.Pane(msg)),
    Feature_SCM.Contributions.commands(
      ~isFocused={
        focus == Focus.SCM;
      },
      state.scm,
    )
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.SCM(msg)),
    Feature_SideBar.Contributions.commands
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.SideBar(msg)),
  ]);
};
