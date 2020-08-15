/*
 * A workspace models the current 'ambient environment' of the editor, in particular:
 * - Current directory
 *
 * ...and eventually
 * - Open editors
 * - Local modifications (hot exit)
 * - Per-workspace configuration
 */

type t = {
  workingDirectory: string,
  rootName: string,
};

let initial = workingDirectory => {
  workingDirectory,
  rootName: Filename.basename(workingDirectory),
};
