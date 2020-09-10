include Model;

[@deriving show]
type model = Model.t;

module View = View;

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let textInputFocus = bool("textInputFocus", _ => true);
};

module Contributions = {
    let contextKeys = ContextKeys.[
        textInputFocus
    ];
};
