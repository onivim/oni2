type t;

let create:
  (
    ~fontFamily: Revery.Font.Family.t,
    ~fontWeight: Revery.Font.Weight.t,
    ~fontSize: float,
    ~features: list(Harfbuzz.feature),
    ~smoothing: Revery.Font.Smoothing.t
  ) =>
  t;

let measure: (Uchar.t, t) => float;
