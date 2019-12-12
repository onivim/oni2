/*
 * Diagnostics.rei
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open EditorCoreTypes;
open Oni_Core;

type t;

/*
 * Create an instance of a diagnostics container
 */
let create: unit => t;

/*
 * Change diagnostics for a buffer+diagnostic key pair
 */
let change: (t, Uri.t, string, list(Diagnostic.t)) => t;

/*
 * [clear(diagnostics, key)] removes diagnostics with the key named [key] across all buffers
 */
let clear: (t, string) => t;

/*
 * Get all diagnostics for a buffer
 */
let getDiagnostics: (t, Buffer.t) => list(Diagnostic.t);
let getDiagnosticsAtPosition: (t, Buffer.t, Location.t) => list(Diagnostic.t);
let getDiagnosticsMap: (t, Buffer.t) => IntMap.t(list(Diagnostic.t));
