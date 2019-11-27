/*
 * MessageType.re
 *
 */

type t = int;

let initialized = 0;
let ready = 1;
let initData = 2;
let terminate = 3;
let requestJsonArgs = 4;
let requestJsonArgsWithCancellation = 5;
let acknowledged = 8;
let replyOkJson = 12;
let replyErrError = 13;

let toInt = v => v;
