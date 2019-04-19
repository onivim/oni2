/*
 * Diagnostics.re
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open Oni_Core;
open Oni_Core.Types;

module Diagnostic {
    type t = {
        range: Range.t,
    }

    let create = (
        ~range: Range.t,
        ()
    ) => {
        let ret: t = { range: range };
        ret;
    };
};

/*
 * The type for diagnostics is a nested map:
 * - First level: Buffer Path
 * - Second level: Diagnostic identifier / key. 
 *   For example - TypeScript might have keys for both compiler errors and lint warnings
 * - Diagnostic list corresponding to the buffer, key pair
 */
type t = StringMap.t(StringMap.t(list(Diagnostic.t)))

let create = () => StringMap.empty;

let change = (instance, _buffer, _diagName, _diagnostics) => {
    instance
};

let getDiagnostics = (_instance, _buffer) => [];

