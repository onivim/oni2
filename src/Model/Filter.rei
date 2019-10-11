/*
 * Filter.rei
 *
 * Module to filter & rank items using various strategies.
 */

let rank: (string, list(Actions.menuCommand)) => list(Actions.menuCommand);
let formatName: (Actions.menuCommand, bool) => string;
