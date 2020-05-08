/*
 * ExtHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;

module Core = Oni_Core;

module SCM = ExtHostClient_SCM;

type msg =
  | SCM(SCM.msg);
