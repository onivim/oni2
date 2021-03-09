module Schema = Schema;
module View = View;

include Model;

// SUBSCRIPTION

let sub = _model => Isolinear.Sub.none;

// CONTRIBUTIONS

module Contributions = {
  let contextKeys = Model.contextKeys;
};
