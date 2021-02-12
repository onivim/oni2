[@deriving show]
type t = {
  control: bool,
  alt: bool,
  altGr: bool,
  shift: bool,
  super: bool,
};

let none = {
  control: false,
  alt: false,
  altGr: false,
  shift: false,
  super: false,
};

let equals = (mod1, mod2) => {
  mod1.control == mod2.control
  && mod1.alt == mod2.alt
  && mod1.altGr == mod2.altGr
  && mod1.shift == mod2.shift
  && mod1.super == mod2.super;
};
