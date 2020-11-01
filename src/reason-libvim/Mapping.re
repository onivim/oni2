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

type scriptId = int;

type t = {
  mode,
  fromKeys: string, // mapped from, lhs
  toValue: string, // mapped to, rhs
  expression: bool,
  recursive: bool,
  silent: bool,
  scriptId: scriptId,
};
