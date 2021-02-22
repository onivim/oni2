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

module Instance = {
  type t('outmsg) = {schema: Schema.menu('outmsg)};
};

type model('outmsg) = {menus: list(Schema.menu('outmsg))};

let initial = {menus: []};

//let isMenuOpen = ({menus}) => menus != [];
let isMenuOpen = _ => true;

let show = (~menu, model) => {menus: [menu, ...model.menus]};

// UPDATE

type outmsg('action) =
  | Action('action)
  | Nothing;

let update = (msg, model) => {
  prerr_endline("MSG: " ++ show_msg(msg));
  (model, Nothing);
};
