type t = {isOpen: bool};

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | ShowHome => {isOpen: true}
  | ShowEditor => {isOpen: false}
  | _ => state
  };

let create = () => {isOpen: true};
