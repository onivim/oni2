type item = {
  label: string,
  msg: Actions.t,
  condition: WhenExpr.t,
};

type t = {
  id: string,
  teims: list(item),
};
