module Internal = {
  let generateId = {
    let lastId = ref(0);

    () => {
      lastId := lastId^ + 1;
      lastId^;
    };
  };
};

// MODEL

[@deriving show({with_path: false})]
type kind =
  | Success
  | Info
  | Warning
  | Error;

[@deriving show({with_path: false})]
type notification = {
  id: int,
  kind,
  message: string,
};

type model = list(notification);

let initial = [];

// UPDATE

[@deriving show({with_path: false})]
type msg =
  | Created(notification)
  | Dismissed(notification)
  | AllDismissed;

let update = (model, msg) => {
  switch (msg) {
  | Created(item) => [item, ...model]
  | Dismissed(item) => List.filter(it => it.id != item.id, model)
  | AllDismissed => initial
  };
};

let oldest = model => model |> List.rev |> List.hd; // NOTE: raises exception if empty

module Effects = {
  let create = (~kind=Info, message) =>
    Isolinear.Effect.createWithDispatch(~name="notification.create", dispatch =>
      dispatch(Created({id: Internal.generateId(), kind, message}))
    );

  let dismiss = notification =>
    Isolinear.Effect.createWithDispatch(~name="notification.dismiss", dispatch =>
      dispatch(Dismissed(notification))
    );

  let dismissAll =
    Isolinear.Effect.createWithDispatch(
      ~name="notification.dismissAll", dispatch =>
      dispatch(AllDismissed)
    );
};
