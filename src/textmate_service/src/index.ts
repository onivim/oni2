/*
 * index.ts
 *
 * Entry for the external textmate tokenizer service
 */

import * as fs from "fs"
import * as rpc from "vscode-jsonrpc"
import * as vsctm from "vscode-textmate"
import * as Buffer from "./Buffer"
import * as Job from "./Job"
import * as Protocol from "./Protocol"
import * as Tokenization from "./Tokenization"

let connection = rpc.createMessageConnection(
    new rpc.StreamMessageReader(process.stdin),
    new rpc.StreamMessageWriter(process.stdout),
)

interface ITextmateInitData {
    [scope: string]: string
}

let initializeNotification = new rpc.NotificationType<ITextmateInitData, void>("initialize")
let initializedNotification = new rpc.NotificationType<string, void>("initialized")
let exitNotification = new rpc.NotificationType<string, void>("exit")

interface IBufferUpdate {
    id: number
    startLine: number
    endLine: number
    lines: string[]
    version: number
}

type BufferUpdateParams = [string, IBufferUpdate]

let textmateBufferUpdate = new rpc.NotificationType<BufferUpdateParams, void>(
    "textmate/bufferUpdate",
)

let textmateTokenNotification = new rpc.NotificationType<Protocol.PublishTokenParams, void>(
    "textmate/publishTokens",
)

let textmateGrammarPreloadNotification = new rpc.NotificationType<string, string>(
    "textmate/preloadScope",
)
let textmateGrammarLoadedNotification = new rpc.NotificationType<string, void>(
    "textmate/scopeLoaded",
)

type tokenResult = [number, number, string[]]

interface ITokenizeLineRequestParams {
    scopeName: string
    line: string
}

interface ITokenizeLineResponse {
    tokens: tokenResult[]
}

interface ISetThemeRequestParams {
    path: string
}

let textmateTokenizeLineRequest = new rpc.RequestType<
    ITokenizeLineRequestParams,
    ITokenizeLineResponse,
    string,
    {}
>("textmate/tokenizeLine")
let textmateSetThemeRequest = new rpc.RequestType<ISetThemeRequestParams, string[], string, {}>(
    "textmate/setTheme",
)

let grammarPaths: ITextmateInitData = {}

const registry = new vsctm.Registry({
    loadGrammar: function(scopeName) {
        var path = grammarPaths[scopeName]
        if (path) {
            return new Promise((c, e) => {
                fs.readFile(path, (error, content) => {
                    if (error) {
                        e(error)
                    } else {
                        var rawGrammar = vsctm.parseRawGrammar(content.toString(), path)
                        connection.sendNotification("textmate/scopeLoaded", scopeName)
                        c(rawGrammar)
                    }
                })
            })
        } else {
            return <any>null
        }
    },
})

connection.listen()

connection.onNotification(textmateGrammarPreloadNotification, scope => {
    registry.loadGrammar(scope)
})

connection.onNotification(initializeNotification, (paths: ITextmateInitData) => {
    grammarPaths = paths
    connection.sendNotification(initializedNotification, {})
})

connection.onNotification(exitNotification, () => {
    process.exit(0)
});

let idToBuffer: {[id: number]: Buffer.Buffer } = {};
let jobManager = new Job.JobManager();

connection.onNotification(textmateBufferUpdate, params => {
    let [scope, bufferUpdate] = params

    // Get current buffer
    let buffer = idToBuffer[bufferUpdate.id] || Buffer.create(bufferUpdate.id, [], -1);
    let newBuffer = Buffer.update(buffer, bufferUpdate);
    idToBuffer[bufferUpdate.id] = newBuffer;

    Buffer.print(newBuffer);

    // Create high priority job to do update
    // let newJob = TokenizeJob(newBuffer, bufferUpdate.startLine, 50, tokens, keywords)
    // get current rule stack 
    // report at end
    // high-pri constant (50)
    // low-pri constant (250)

    // Just do initial update for now..
    // TODO: Handle incremental updates
    if (bufferUpdate.startLine === 0 && bufferUpdate.endLine === -1) {
        registry.loadGrammar(scope).then(grammar => {
            const lines = bufferUpdate.lines
            const ret = []
            let ruleStack: any = null

            // TODO:
            // Do we need to break up / chunk this?
            // How does it scale up with 10k line buffers?
            for (var i = 0; i < lines.length; i++) {
                const r = grammar.tokenizeLine2(lines[i], ruleStack)
                const tokens = Array.prototype.slice.call(r.tokens)
                ruleStack = r.ruleStack
                ret[i] = {
                    line: i,
                    tokens: tokens,
                };
            }

            connection.sendNotification(textmateTokenNotification, {
                bufferId: bufferUpdate.id,
                version: bufferUpdate.version,
                lines: ret,
            })
        })
    }
})

connection.onRequest<ITokenizeLineRequestParams, ITokenizeLineResponse, string, {}>(
    textmateTokenizeLineRequest,
    params => {
        return registry.loadGrammar(params.scopeName).then(grammar => {
            const tokens = grammar.tokenizeLine(params.line, <any>null)

            if (!tokens || !tokens.tokens) {
                return {
                    tokens: [],
                }
            } else {
                const colors = grammar.tokenizeLine2(params.line, <any>null)
                const parsedTokens = tokens.tokens
                const filteredTokens = parsedTokens.filter(t => t.scopes.length > 1)
                const result: tokenResult[] = filteredTokens.map(
                    t => [t.startIndex, t.endIndex, t.scopes] as tokenResult,
                )

                const colorTokens = Array.prototype.slice.call(colors.tokens)
                return {
                    tokens: result,
                    colors: colorTokens,
                }
            }
        })
    },
)

connection.onRequest<ISetThemeRequestParams, string[], string, {}>(
    textmateSetThemeRequest,
    params => {
        let themeFile = fs.readFileSync(params.path)
        let parsedTheme = JSON.parse(themeFile.toString("utf8"))

        let rawTheme: vsctm.IRawTheme = {
            name: parsedTheme.name,
            settings: parsedTheme.tokenColors || [],
        }

        registry.setTheme(rawTheme)
        const precolors = registry.getColorMap()
        const colors = precolors.filter(c => !!c)
        return colors
    },
)
