[@deriving show]
type msg =
  | KeyPressed(string)
  | Pasted(string)
  | Input(Component_InputText.msg);

module Msg = {
  let keyPressed = key => KeyPressed(key);
  let pasted = key => Pasted(key);
};

type progress =
  | Loading
  | InProgress(float)
  | Complete;

type model('outmsg) = {menus: list(Schema.Instance.t('outmsg))};

let initial = {
  menus: [],
  //Schema.menu([]) |> Schema.instantiate
};

let isMenuOpen = ({menus}) => menus != [];

let show = (~menu, model) => {
  menus: [Schema.instantiate(menu), ...model.menus],
};

let focus = (~index, model) => {
  prerr_endline("!! FOCUS");
  model;
};

let next = model => {
  prerr_endline("!! NEXT");
  model;
};

let prev = model => {
  prerr_endline("!! PREV");
  model;
};

let cancel = model => {
  prerr_endline("!! CANCEL");
  model;
};

let select = model => {
  prerr_endline("!! SELECT");
  (model, Isolinear.Effect.none);
};

let current = model => {
  switch (model.menus) {
  | [] => None
  | [hd, ..._] => Some(hd)
  };
};

// UPDATE

type outmsg('action) =
  | Action('action)
  | Nothing;

let updateCurrentMenu = (f, model) => {
  let menus' =
    switch (model.menus) {
    | [] => []
    | [current, ...others] => [f(current), ...others]
    };
  {...model, menus: menus'};
};

let update = (msg, model) => {
  Schema.Instance.(
    switch (msg) {
    | Pasted(text) => (
        model |> updateCurrentMenu(Schema.Instance.paste(~text)),
        Nothing,
      )

    | KeyPressed(key) => (
        model |> updateCurrentMenu(Schema.Instance.key(~key)),
        Nothing,
      )

    | Input(msg) => (
        model |> updateCurrentMenu(Schema.Instance.input(msg)),
        Nothing,
      )
    }
  );
};

let contextKeys = model => {
  WhenExpr.ContextKeys.(
    [
      Schema.bool("listFocus", isMenuOpen),
      Schema.bool("inQuickOpen", isMenuOpen),
    ]
    |> Schema.fromList
    |> fromSchema(model)
  );
};
