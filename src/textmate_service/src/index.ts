/*
 * index.ts
 *
 * Entry for the external textmate tokenizer service
 */


import * as rpc from "vscode-jsonrpc";

let connection = rpc.createMessageConnection(new rpc.StreamMessageReader(process.stdin), new rpc.StreamMessageWriter(process.stdout));

let notification = new rpc.NotificationType<string, void>('textmateServer/init');

connection.listen();

connection.sendNotification(notification, 'Hello');
