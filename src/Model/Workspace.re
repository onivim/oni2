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
  rootDir: string,
};

type t = option(workspace);

type empty: t = None;

let reducer = (v: t, a) => {
  switch (a) {
  | Actions.OpenExplorer(dir) => Some({
    workingDirectory: dir,
    rootDir: Filename.basename(dir), 
  })
  | _ => v 
  }
};

