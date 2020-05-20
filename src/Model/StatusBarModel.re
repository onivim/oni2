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
  | NotificationsContextMenu;

module Item = {
  type t = {
    id: string,
    priority: int,
    label: Exthost.Label.t,
    alignment: Exthost.Msg.StatusBar.alignment,
  };

  let create = (~id, ~priority, ~label, ~alignment=Left, ()) => {
    id,
    priority,
    label,
    alignment,
  };
};

type t = list(Item.t);

let create: unit => t = () => [];
