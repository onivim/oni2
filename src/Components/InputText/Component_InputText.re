include Model;

[@deriving show]
type model = Model.t;

module View = View;

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let textInputFocus = bool("textInputFocus", isFocused);
};

module Contributions = {
  let contextKeys = model => {
    open WhenExpr.ContextKeys
  let schema = ContextKeys.[textInputFocus]
  |> Schema.fromList
  fromSchema(schema, model);
  };
};
