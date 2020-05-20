/*
 * StatusBar
 *
 * Model for status bar items
 */
open Exthost.Msg.StatusBar;

[@deriving show({with_path: false})]
type action =
  | DiagnosticsClicked
  | NotificationClearAllClicked
  | NotificationCountClicked
  | NotificationsContextMenu
  | ContributedItemClicked({id: string, command: string});

module Item = {
  type t = {
    id: string,
    priority: int,
    label: Exthost.Label.t,
    alignment: Exthost.Msg.StatusBar.alignment,
    command: option(string),
  };

  let create = (~command=?, ~id, ~priority, ~label, ~alignment=Left, ()) => {
    id,
    priority,
    label,
    alignment,
    command,
  };
};

type t = list(Item.t);

let create: unit => t = () => [];
