type t = {
  control: bool,
  alt: bool,
  altGr: bool,
  shift: bool,
  meta: bool,
};

let none = {
  control: false,
  alt: false,
  altGr: false,
  shift: false,
  meta: false,
};

let equals = (mod1, mod2) => {
  mod1.control == mod2.control
  && mod1.alt == mod2.alt
  && mod1.altGr == mod2.altGr
  && mod1.shift == mod2.shift
  && mod1.meta == mod2.meta;
};
