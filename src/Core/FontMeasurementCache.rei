type t;

let create:
  (
    ~fontFamily: Revery.Font.Family.t,
    ~fontSize: float,
    ~features: list(Harfbuzz.feature),
    ~smoothing: Revery.Font.Smoothing.t
  ) =>
  t;

let measure: (Uchar.t, t) => float;
