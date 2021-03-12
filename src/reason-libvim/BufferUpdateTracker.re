module Internal = {
  let shouldAdjustCursors = ref(false);
};

let watch = (~shouldAdjustCursors, f) => {
  let prev = Internal.shouldAdjustCursors^;
  Internal.shouldAdjustCursors := shouldAdjustCursors;
  f();
  Internal.shouldAdjustCursors := prev;
  ();
};
