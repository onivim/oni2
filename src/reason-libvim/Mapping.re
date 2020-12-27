[@deriving show]
type mode =
  | Insert // imap, inoremap
  | Language // lmap
  | CommandLine // cmap
  | Normal // nmap, nnoremap
  | VisualAndSelect // vmap, vnoremap
  | Visual // xmap, xnoremap
  | Select // smap, snoremap
  | Operator // omap, onoremap
  | Terminal // tmap, tnoremap
  | InsertAndCommandLine // :map!
  | All; // :map;

module ScriptId = {
  [@deriving show]
  type t = int;

  let default = 0;

  let toInt: t => int = Fun.id;
};

let defaultScriptId = 0;
[@deriving show]
type t = {
  mode,
  fromKeys: string, // mapped from, lhs
  toValue: string, // mapped to, rhs
  expression: bool,
  recursive: bool,
  silent: bool,
  scriptId: ScriptId.t,
};
