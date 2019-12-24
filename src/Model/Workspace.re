/*
 * A workspace models the current 'ambient environment' of the editor, in particular:
 * - Current directory
 *
 * ...and eventually
 * - Open editors
 * - Local modifications (hot exit)
 * - Per-workspace configuration
 */

type workspace = {
  workingDirectory: string,
  rootName: string,
};

type t = option(workspace);

let initial: t = None;
