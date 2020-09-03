open EditorCoreTypes;

let validateRange =
    (
      expect: RelyInternal__Test.matchers(unit),
      actual: Range.t,
      expected: Range.t,
    ) => {
  expect.int((actual.start.line :> int)).toBe((expected.start.line :> int));
  expect.int((actual.stop.line :> int)).toBe((expected.stop.line :> int));
  expect.int((actual.start.column :> int)).toBe(
    (expected.start.column :> int),
  );
  expect.int((actual.stop.column :> int)).toBe(
    (expected.stop.column :> int),
  );
};

let validateRanges =
    (expect: RelyInternal__Test.matchers(unit), actual, expected) => {
  List.iter2(validateRange(expect), actual, expected);
};
