type t = {isOpen: bool};

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | ShowHome => {isOpen: true}
  | _ => state
  };

let create = () => {isOpen: true};
