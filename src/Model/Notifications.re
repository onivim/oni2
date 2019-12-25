open Actions;

type t = list(Notification.t);

let initial: t = [];

let reduce = (state, action: Actions.t) => {
  switch (action) {
  | ShowNotification(item) => [item, ...state]
  | HideNotification(item) => List.filter(it => it != item, state)
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
