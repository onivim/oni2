open Actions;
open Notification;

type t = list(Notification.t);

module ContextMenu =
  Oni_Components.ContextMenu.Make({});

let initial: t = [];

let reduce = (state, action: Actions.t) => {
  switch (action) {
  | ShowNotification(item) => [item, ...state]
  | HideNotification(item) => List.filter(it => it.id != item.id, state)
  | ClearNotifications => initial
  | _ => state
  };
};

let any = (state: t) => {
  switch (state) {
  | [] => false
  | _ => true
  };
};

let getOldest = (state: t) => state |> List.rev |> List.hd;
