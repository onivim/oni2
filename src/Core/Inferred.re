type t('a) =
  | Implicit('a)
  | Explicit('a);

let implicit = v => Implicit(v);
let explicit = v => Explicit(v);

let isImplicit =
  fun
  | Implicit(_) => true
  | Explicit(_) => false;

let isExplicit =
  fun
  | Implicit(_) => false
  | Explicit(_) => true;

let map = f =>
  fun
  | Implicit(orig) => Implicit(f(orig))
  | Explicit(orig) => Explicit(f(orig));

let update = (~new_, original) => {
  switch (new_, original) {
  | (Explicit(_) as newExplicit, _) => newExplicit
  | (_, Explicit(_) as originalExplicit) => originalExplicit
  | (Implicit(_) as newInferred, Implicit(_)) => newInferred
  };
};

let flatMap = f =>
  fun
  | Implicit(orig) => f(orig)
  | Explicit(orig) => f(orig);

let value =
  fun
  | Implicit(v) => v
  | Explicit(v) => v;
