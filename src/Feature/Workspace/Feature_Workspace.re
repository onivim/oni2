/*
 * A workspace models the current 'ambient environment' of the editor, in particular:
 * - Current directory
 *
 * ...and eventually
 * - Open editors
 * - Local modifications (hot exit)
 * - Per-workspace configuration
 */

type model = {
  workingDirectory: string,
  openedFolder: option(string),
  rootName: string,
};

let initial = (~openedFolder: option(string), workingDirectory) => {
  workingDirectory,
  openedFolder,
  rootName: Filename.basename(workingDirectory),
};
