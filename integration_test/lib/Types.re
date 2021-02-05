module Model = Oni_Model;

type dispatchFunction = Model.Actions.t => unit;
type runEffectsFunction = unit => unit;
type waiter = Model.State.t => bool;
type waitForState = (~name: string, ~timeout: float=?, waiter) => unit;

type staysTrueFunction = (~name: string, ~timeout: float, waiter) => unit;

type inputFunction = (~modifiers: EditorInput.Modifiers.t=?, string) => unit;

type keyFunction =
  (~modifiers: EditorInput.Modifiers.t=?, EditorInput.Key.t) => unit;

type testContext = {
  dispatch: dispatchFunction,
  wait: waitForState,
  runEffects: runEffectsFunction,
  input: inputFunction,
  key: keyFunction,
  staysTrue: staysTrueFunction,
};

type testCallback = testContext => unit;
