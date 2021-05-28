module Make:
  () =>
   {
    type t;

    let create: (~friendlyName: string) => t;

    let compare: (t, t) => int;
    let toString: t => string;
  } =
  (()) => {
    let nextId: ref(Int64.t) = ref(Int64.zero);

    type t = (string, Int64.t);

    let create = (~friendlyName) => {
      let id = nextId^;
      nextId := Int64.add(id, Int64.one);

      (friendlyName ++ Int64.to_string(id), id);
    };

    let compare = (a, b) => {
      Int64.compare(snd(a), snd(b));
    };

    let toString = fst;
  };
