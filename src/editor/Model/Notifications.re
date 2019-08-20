open Oni_Core;
open Oni_Core.Types;

open Actions;

type t = list(Notification.t);

let default: t = [
  Notification.create(
    ~notificationType=Error,
    ~message="This is a test notification",
    ~title="Hello",
    (),
  ),
  Notification.create(
    ~notificationType=Warning,
    ~message="This is a test notification",
    ~title="Hello",
    (),
  ),
  Notification.create(
    ~notificationType=Info,
    ~message="This is a test notification",
    ~title="Hello",
    (),
  ),
  Notification.create(
    ~notificationType=Success,
    ~message=
      "This is a test notification2, that is really long, and maybe should've used lorem ipsum text but didn't so here we are",
    ~title="World",
    (),
  ),
];

let reduce = (state: t, action: Actions.t) => {
print_endline ("reducer called");
  switch (action) {
  | ShowNotification(n) => [n, ...state]
  | HideNotification(id) => List.filter(n => n.id != id, state)
  | _ => state
  };
};
