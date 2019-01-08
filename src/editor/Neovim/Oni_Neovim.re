/*
 * Oni_Neovim.re
 *
 * Top-level module for Oni_Neovim. This is the entry point for interfacing with Neovim.
 */

/*
 * Some notes:
 *
 * Today, we are using the `msgpack` API, but long-term, we'd like to pursue an in-proc
 * model using a linked `libnvim`. This will reduce interop overhead as well as allowed
 * the ability to share buffer stroage, instead of duplicating it on Oni2's side.
 */

module NeovimProcess = NeovimProcess;
