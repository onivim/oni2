type command = {
  name: string,
  action: unit => Actions.t,
};

type t = {
  isOpen: bool,
  commands: list(command),
};

let create = () => {isOpen: false, commands: []};
