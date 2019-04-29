/*
 * Diagnostics.rei
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open Oni_Core;

module Diagnostic: {
  type t = {range: Range.t};

  let create: (~range: Range.t, unit) => t;
};

type t;

/*
 * Create an instance of a diagnostics container
 */
let create: unit => t;

/*
 * Change diagnostics for a buffer+diagnostic key pair
 */
let change: (t, Buffer.t, string, list(Diagnostic.t)) => t;

/*
 * Get all diagnostics for a buffer
 */
let getDiagnostics: (t, Buffer.t) => list(Diagnostic.t);
