/*
 * NeovimProtocol.re
 *
 * Wrapper on-top of the NeovimApi primitives to hide the Msgpack details.
 */

/* open Oni_Core; */
open Rench;

type notification = 
| ModeChanged(string);

/* 
 * Simple API for interacting with Neovim,
 * abstracted from Msgpck
 */
type t = {

  uiAttach: unit => unit,
  input: string => unit,

  /* TODO */
  /* Typed notifications */
  onNotification: Event.t(notification),
};

/* TODO */
