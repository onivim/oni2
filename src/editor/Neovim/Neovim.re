/*
 * Neovim.re
 *
 * Interface for working with `nvim` in the Oni2 context.
 *
 * Architecturally, the design for how we talk to `nvim` in Oni2 is much different
 * than how we talked to `nvim` in Oni1. In Oni1, `nvim` was responsible for essentially
 * the entire 'editing world' - aside from the language integration w/ LSP.
 *
 * In Oni2, more of the editing responsibility is shifting to the 'Oni' front-end.
 *
 * Here's the breakdown of responsibilities as I'd see it for Oni2:
 *
 * `nvim` responsibilities:
 * - Tab management
 * - Buffer management
 * - Normal Mode / Ex Mode / Visual Mode
 *
 * `Oni2` responsibilities:
 * - Insert mode (tight inetgration with language services)
 * - View rendering (line wrapping, smooth scrolling, minimap, etc)
 * - Window management (VSCode/Atom style)
 * - LSP integration (via VSCode extension host)
 * - Debug integration
 *
 * This shift in responsibilities allows us to provide a first-class insert-mode experience,
 * and enables new capabilities in terms of rendering UI in our editor in a performant way.
 *
 * The current exploration of the interface with Neovim is that we send specific high-level gestures:
 * - key presses
 * - editor actions (open buffer)
 *
 * And we treat neovim as an abstraction of an `(EditorState, Action) => EditorState` - essentially treating `nvim` as a pure function of EditorState -> EditorState.
 * This model isn't perfect, as there are events that will be triggered out-of-band, but I think it will be close enough to get us far - and give us a clean,
 * testable layer as a foundation for interacting with the other pieces in the Oni2 architecture.
 */

type t = {
    pid: int,
};


let start = (
    ~_neovimPath: string,
) => {

    /* TODO */
    ();
};

