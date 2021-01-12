module Model = Oni_Model;

type dispatchFunction = Model.Actions.t => unit;
type runEffectsFunction = unit => unit;
type waiter = Model.State.t => bool;
type waitForState = (~name: string, ~timeout: float=?, waiter) => unit;

type inputFunction = string => unit;

type testCallback =
  (dispatchFunction, waitForState, runEffectsFunction) => unit;

type testCallbackWithInput =
  (inputFunction, dispatchFunction, waitForState, runEffectsFunction) => unit;
