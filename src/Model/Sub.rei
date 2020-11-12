let activeFile:
  (~id: string, ~state: State.t, ~toMsg: option(string) => Actions.t) =>
  Isolinear.Sub.t(Actions.t);
