/*
 * YankHighlights.re
 *
 * Per-buffer yank / delete highlight info
 */

open Oni_Core;
open Oni_Core.Types;

type yankHighlights = {
  ranges: VisualRange.t,
};

type yankHighlightsTuple = (VisualRange.t);

type t = IntMap.t(yankHighlights);

let create: unit => t = () => IntMap.empty;

let reduce = (state: t, action: Actions.t) => {
    switch(action) {
    | YankBlock(id, v) => 
        IntMap.update(id, (currentValue) => {
            switch (currentValue) {
            | None => Some({ ranges: v })
            | Some(_) => Some({ ranges: v})
            }
        }, state);
    | _ => state;
    }
};