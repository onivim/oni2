/*
 * Command.re
 *
 * Type definitions for commands.
 */

open Actions;

type t = Actions.command;

let create =
    (~category=None, ~enabled=_ => true, ~icon=None, ~name, ~action, ()) =>
  Actions.{
    commandCategory: category,
    commandName: name,
    commandAction: action,
    commandEnabled: enabled,
    commandIcon: icon,
  };

let toQuickMenu: t => Actions.menuItem =
  v =>
    Actions.{
      category: v.commandCategory,
      name: v.commandName,
      command: () => v.commandAction,
      icon: v.commandIcon,
      highlight: [],
    };

let isEnabled = v => v.commandEnabled();

let getAction = v => v.commandAction;
