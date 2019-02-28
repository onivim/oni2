/*
 * index.ts
 *
 * Entry for the external textmate tokenizer service
 */


import * as rpc from "vscode-jsonrpc";

let connection = rpc.createMessageConnection(new rpc.StreamMessageReader(process.stdin), new rpc.StreamMessageWriter(process.stdout));

let initializeNotification = new rpc.NotificationType<string, void>('initialize');
let initializedNotification = new rpc.NotificationType<string, void>('initialized');
let exitNotification = new rpc.NotificationType<string, void>('exit');

connection.listen();

console.error("HELLO");

connection.onNotification(initializeNotification, () => {
    console.error("GOT INITIALIZE");
    connection.sendNotification(initializedNotification, {});  
});

connection.onNotification(exitNotification, () => {
    process.exit(1);
});
