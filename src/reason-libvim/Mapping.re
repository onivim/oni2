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

type rhs =
  | Keys(string) // Mapped to a new sequence of keys
  | Expression(string); // Mapped to an expression - <expr> was used;

type scriptContext = {id: int};

type t = {
  mode,
  fromKeys: string, // mapped from, lhs
  toKeys: rhs, // mapped to, rhs
  recursive: bool,
  silent: bool,
  scriptContext,
};
