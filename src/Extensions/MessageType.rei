/*
 * MessageType.rei
 */

type t;
let initialized: t;
let ready: t;
let initData: t;
let terminate: t;
let requestJsonArgs: t;
let requestJsonArgsWithCancellation: t;
let acknowledged: t;
let replyOkJson: t;
let replyErrError: t;

let toInt: t => int;
