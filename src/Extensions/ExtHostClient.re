/*
 * ExtHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

module SCM = ExtHostClient_SCM;

type msg =
  | SCM(SCM.msg);
