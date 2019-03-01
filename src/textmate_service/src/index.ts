/*
 * index.ts
 *
 * Entry for the external textmate tokenizer service
 */

import * as fs from "fs";
import * as rpc from "vscode-jsonrpc";
import * as vsctm from "vscode-textmate";

let connection = rpc.createMessageConnection(new rpc.StreamMessageReader(process.stdin), new rpc.StreamMessageWriter(process.stdout));

interface ITextmateInitData {
    [scope: string]: string;
};

let initializeNotification = new rpc.NotificationType<ITextmateInitData, void>('initialize');
let initializedNotification = new rpc.NotificationType<string, void>('initialized');
let exitNotification = new rpc.NotificationType<string, void>('exit');

let textmateGrammarPreloadNotification = new rpc.NotificationType<string, string>('textmate/preloadScope');
let textmateGrammarLoadedNotification = new rpc.NotificationType<string, void>('textmate/scopeLoaded');

type tokenResult = [number, number, string[]];

interface ITokenizeLineRequestParams {
    scopeName: string;
    line: string;
}

let textmateTokenizeLineRequest = new rpc.RequestType<ITokenizeLineRequestParams, tokenResult[], string, {}>('textmate/tokenizeLine');

let grammarPaths: ITextmateInitData = {};

const registry = new vsctm.Registry({
        loadGrammar: function (scopeName) {
            var path = grammarPaths[scopeName];
            if (path) {
                return new Promise((c,e) => {
                    fs.readFile(path, (error, content) => {
                        if (error) {
                            e(error);
                        } else {
                            var rawGrammar = vsctm.parseRawGrammar(content.toString(), path);
                            connection.sendNotification("textmate/scopeLoaded", scopeName);
                            c(rawGrammar);
                        }
                    });
                });
            } else  {
                return <any>null;
            }
        }
    });

connection.listen();

connection.onNotification(textmateGrammarPreloadNotification, (scope) => {
    registry.loadGrammar(scope);
});

connection.onNotification(initializeNotification, (paths: ITextmateInitData) => {
    grammarPaths = paths;
    connection.sendNotification(initializedNotification, {});
});

connection.onNotification(exitNotification, () => {
    process.exit(0);
});

connection.onRequest<ITokenizeLineRequestParams, tokenResult[], string, {}>(textmateTokenizeLineRequest, (params) => {
    return registry.loadGrammar(params.scopeName).then((grammar) => {
       const tokens = grammar.tokenizeLine(params.line, <any>null);

        if (!tokens || !tokens.tokens) {
            return [];
        } else {
            const parsedTokens = tokens.tokens;
            const filteredTokens = parsedTokens.filter((t) => t.scopes.length > 1);
            const result: tokenResult[] = filteredTokens.map((t) => [t.startIndex, t.endIndex, t.scopes] as tokenResult);
            console.error(JSON.stringify(result));
            return result;
        }

    });
})
