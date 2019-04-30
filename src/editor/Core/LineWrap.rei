/**
 * LineWrap.rei
 *
 * Logic for wrapping lines in Onivim 2
 */
open Types;

/*
 * Opaque type containing 'virtual lines'
 */
type t;

/*
 * Construct virtual line info based on a string and a wrap point
 */
let create: (string, int) => t;

/*
 * Return the number of virtual lines
 */
let count: t => int;

/*
 * Convert an index into a (line, character) Position.t,
 * based on the virtual line information.
 */
let toVirtualPosition: (Index.t, t) => Position.t;

let getOffsets: (int, t) => (int, int);
