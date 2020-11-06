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

[@deriving show]
type scriptId = int;

let defaultScriptId = 0;
[@deriving show]
type t = {
  mode,
  fromKeys: string, // mapped from, lhs
  toValue: string, // mapped to, rhs
  expression: bool,
  recursive: bool,
  silent: bool,
  scriptId,
};
