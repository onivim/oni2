open Actions;
open Notification;

type t = list(Notification.t);

let initial: t = [];

let reduce = (state, action: Actions.t) => {
  switch (action) {
  | Font(Service_Font.FontLoadError(msg)) =>
    let errorNotification = Notification.create(~kind=Error, msg);
    [errorNotification, ...state];
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
