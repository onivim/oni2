/*
 * CommandManager.re
 *
 * Top-level state of the editor
 */

open Oni_Core;

let current = {
  // Static commands are not dependent on the state
  let static =
    Command.Lookup.unionMany([
      Feature_Configuration.Contributions.commands
      |> Command.Lookup.fromList
      |> Command.Lookup.map(msg => Actions.Configuration(msg)),
      Feature_Buffers.Contributions.commands
      |> Command.Lookup.map(msg => Actions.Buffers(msg)),
      Feature_Logging.Contributions.commands
      |> Command.Lookup.fromList
      |> Command.Lookup.map(msg => Actions.Logging(msg)),
      Feature_SideBar.Contributions.commands
      |> Command.Lookup.fromList
      |> Command.Lookup.map(msg => Actions.SideBar(msg)),
      Feature_Help.Contributions.commands
      |> Command.Lookup.fromList
      |> Command.Lookup.map(msg => Actions.Help(msg)),
      Feature_Zen.Contributions.commands
      |> Command.Lookup.fromList
      |> Command.Lookup.map(msg => Actions.Zen(msg)),
    ]);

  state => {
    let focus = FocusManager.current(state);
    Command.Lookup.unionMany([
      static,
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
      Feature_Workspace.Contributions.commands(state.workspace)
      |> Command.Lookup.fromList
      |> Command.Lookup.map(msg => Actions.Workspace(msg)),
    ]);
  };
};
