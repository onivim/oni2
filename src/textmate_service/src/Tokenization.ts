/*
 * Tokenization.ts
 *
 * Data type for per-buffer tokenization info
 */

import * as vsctm from "vscode-textmate"
import * as Job from "./Job"
import * as Protocol from "./Protocol"
import * as Buffer from "./Buffer"

export interface TokenizedLine {
    version: number,
    ruleStack2: vsctm.StackElement;
};

export interface TokenizedBuffer {
    [key: number]: TokenizedLine
}

export interface TokenizationResult2 {
    line: number,
    tokens: number[]
    ruleStack2: vsctm.StackElement,
}

export interface TokenizationStore {
    getRuleStack: (bufferId: number, line: number) => vsctm.StackElement;

    commitUpdates: (bufferId: number, result: TokenizationResult2[]) => void;
}

export class TokenizationJob implements Job.Job {
    
    constructor(
        private buffer: Buffer.Buffer, 
        private startLine: number, 
        private count: number, 
        private grammar: vsctm.IGrammar,
        private store: TokenizationStore,
        public  priority: number) {
    }

    public execute() {

        let ruleStack2 = this.store.getRuleStack(this.buffer.id, this.startLine - 1);

        let idx = this.startLine;
        let done = false;
        let resultsToSend: TokenizationResult2[] = [];
        while (idx <  this.startLine + this.count && this.startLine + idx < this.buffer.lines.length) {

            let currentRuleStack = this.store.getRuleStack(this.buffer.id, idx);

            if (currentRuleStack === ruleStack2) {
                done = true;
                break;
            }

            let currentLine = this.buffer.lines[idx].contents;
            const r = this.grammar.tokenizeLine2(currentLine, ruleStack2);
            const tokens = Array.prototype.slice.call(r.tokens);

            resultsToSend.push({
                line: idx, 
                tokens,
                ruleStack2: r.ruleStack,
            });

            ruleStack2 = r.ruleStack;
        }

        this.store.commitUpdates(this.buffer.id, resultsToSend);

        let ret: Job.Job[] = [];
        if (idx < this.buffer.lines.length && !done) {
           ret = [new TokenizationJob(this.buffer, idx, 500, this.grammar, this.store, 1000)];
        };


        return ret;
    }
}
