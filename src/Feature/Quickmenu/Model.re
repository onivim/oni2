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
  menus:
    [],
      //Schema.menu([]) |> Schema.instantiate
};

let isMenuOpen = ({menus}) => menus != [];

let show = (~menu, model) => {
  menus: [Schema.instantiate(menu), ...model.menus],
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
