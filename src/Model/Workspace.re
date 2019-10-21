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

let empty: t = None;

let reduce = (v: t, a) => {
  switch (a) {
  | Actions.OpenExplorer(dir) => Some({
    workingDirectory: dir,
    rootName: Filename.basename(dir), 
  })
  | _ => v 
  }
};

