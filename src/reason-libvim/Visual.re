open EditorCoreTypes;

let zeroRange =
  Range.create(
    ~start=Location.create(~line=Index.zero, ~column=Index.zero),
    ~stop=Location.create(~line=Index.zero, ~column=Index.zero),
  );

let getRange = () => {
  let (startLine, startColumn, stopLine, stopColumn) =
    Native.vimVisualGetRange();

  // If there is no active range, startLine & stopLine will be '0'
  if (startLine == 0 || stopLine == 0) {
    zeroRange;
  } else {
    Range.create(
      ~start=
        Location.create(
          ~line=Index.fromOneBased(startLine),
          ~column=Index.fromZeroBased(startColumn),
        ),
      ~stop=
        Location.create(
          ~line=Index.fromOneBased(stopLine),
          ~column=Index.fromZeroBased(stopColumn),
        ),
    );
  };
};

let getType = Native.vimVisualGetType;

let onRangeChanged = f => {
  Event.add(f, Listeners.visualRangeChanged);
};
