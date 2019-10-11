open Actions;

type t = list(Notification.t);

let default: t = [];

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | ShowNotification(n) => [n, ...state]
  | HideNotification(id) => List.filter(n => n.id != id, state)
  | _ => state
  };
};

let any = (state: t) => {
  switch (state) {
  | [] => false
  | _ => true
  };
};

let getOldestId = (state: t) => {
  let n = state |> List.rev |> List.hd;

  n.id;
};
