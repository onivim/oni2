include Model;

[@deriving show]
type model = Model.t;

module View = View;

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let textInputFocus = bool("textInputFocus", isFocused);
};

module Contributions = {
  open WhenExpr.ContextKeys;
  let contextKeys = (model: Model.t) => {
    ContextKeys.[textInputFocus] |> Schema.fromList |> fromSchema(model);
  };
};
