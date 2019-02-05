/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;

let reduce: (State.t, Actions.t) => State.t = (s, a) => {
    switch (a) {
    | ChangeMode(m) => { 
        let ret: State.t = {
            mode: m,
        };
        ret;
    }
    | Noop => s
    };
};
