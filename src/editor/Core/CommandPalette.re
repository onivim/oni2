type command = {
  name: string,
  action: unit => Oni_Core.Actions.t,
};

type t = {
  isOpen: bool,
  commands: list(command),
};

let create = () => {isOpen: false, commands: []};
