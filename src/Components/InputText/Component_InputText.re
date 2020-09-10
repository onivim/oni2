include Model;

[@deriving show]
type model = Model.t;

module View = View;

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let textInputFocus = bool("textInputFocus", isFocused);
};

module Contributions = {
  let contextKeys = ContextKeys.[textInputFocus];
};
