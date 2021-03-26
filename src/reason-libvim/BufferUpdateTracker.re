module Internal = {
  let shouldAdjustCursors = ref(false);
};

let shouldAdjustCursors: unit => bool = () => Internal.shouldAdjustCursors^;

let watch = (~shouldAdjustCursors, f) => {
  let prev = Internal.shouldAdjustCursors^;
  Internal.shouldAdjustCursors := shouldAdjustCursors;
  f();
  Internal.shouldAdjustCursors := prev;
  ();
};
