/*
 * Lifecycle.re
 */

type quitCleanup = unit => unit;

type t = {onQuitFunctions: list(quitCleanup)};

let create = () => {onQuitFunctions: []};

let reduce = (state, action) => {
  switch (action) {
  | Actions.RegisterQuitCleanup(f) => {
      onQuitFunctions: [f, ...state.onQuitFunctions],
    }
  | _ => state
  };
};
