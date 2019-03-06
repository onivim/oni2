/*
 * TokenizationStore.ts
 *
 * Holds per-buffer tokenization info
 */

import * as vsctm from "vscode-textmate"
import * as Job from "./Job"
import * as Protocol from "./Protocol"
import * as Buffer from "./Buffer"

export interface TokenizedLine {
    version: number
    ruleStack2: vsctm.StackElement
}

export interface TokenizedBuffer {
    [key: number]: TokenizedLine
}

export interface TokenizationResult2 {
    line: number
    tokens: number[]
    ruleStack2: vsctm.StackElement
}

export interface ITokenizationStore {
    getRuleStack: (bufferId: number, line: number) => vsctm.StackElement

    commitUpdates: (bufferId: number, version: number, result: TokenizationResult2[]) => void
}

export interface CommitTokenInfo {
    line: number
    tokens: number[]
}

export type CommitTokenizationCallback = (bufferId: number, version: number, info: CommitTokenInfo[]) => void

export class TokenizationStore implements ITokenizationStore {
    private _idToTokenState: { [id: number]: TokenizedBuffer } = {}

    constructor(private _commitCallback: CommitTokenizationCallback) {}

    private _getBufferById(id: number): TokenizedBuffer {
        if (!this._idToTokenState[id]) {
            this._idToTokenState[id] = {}
        }

        return this._idToTokenState[id]
    }

    public getRuleStack(bufferId: number, line: number) {
        let buf = this._getBufferById(bufferId)

        let tokenizedLine = buf[line]

        if (!tokenizedLine) {
            return <any>null
        } else {
            return tokenizedLine.ruleStack2
        }
    }

    public commitUpdates(bufferId: number, version: number, result: TokenizationResult2[]) {
        result = result || []

        let buf = this._getBufferById(bufferId)

        result.forEach(v => {
            let curr = buf[v.line]
            buf[v.line] = {
                ...curr,
                ruleStack2: v.ruleStack2,
            }
        })

        let callbackResult: CommitTokenInfo[] = result.map(v => {
            return {
                line: v.line,
                tokens: v.tokens,
            }
        })

        if (callbackResult.length > 0) {
            this._commitCallback(bufferId, version, callbackResult)
        }
    }
}
