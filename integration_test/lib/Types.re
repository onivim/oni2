module Model = Oni_Model;

type dispatchFunction = Model.Actions.t => unit;
type runEffectsFunction = unit => unit;
type waiter = Model.State.t => bool;
type waitForState = (~name: string, ~timeout: float=?, waiter) => unit;
