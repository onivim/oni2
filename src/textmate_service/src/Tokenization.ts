/*
 * Tokenization.ts
 *
 * Data type for per-buffer tokenization info
 */

import * as vsctm from "vscode-textmate"
import * as Job from "./Job"
import * as Protocol from "./Protocol"
import * as Buffer from "./Buffer"
import * as TokenizationStore from "./TokenizationStore"

// Test cases:
// - Commits tokenization
// - Stops after rulestack matches
// - Queues new job if still work left

export class TokenizationJob implements Job.Job {
    constructor(
        private buffer: Buffer.Buffer,
        private startLine: number,
        private count: number,
        private grammar: vsctm.IGrammar,
        private store: TokenizationStore.ITokenizationStore,
        public priority: number,
    ) {}

    public execute() {
        let ruleStack2 = this.store.getRuleStack(this.buffer.id, this.startLine - 1)

        let idx = this.startLine
        let done = false
        let resultsToSend: TokenizationStore.TokenizationResult2[] = []
        while (idx < this.startLine + this.count && idx < this.buffer.lines.length) {
            // console.error(`Running job at line: ${idx} - count: ${this.count} - buffer length: ${this.buffer.lines.length}`);
            let currentRuleStack = this.store.getRuleStack(this.buffer.id, idx)

            // if (currentRuleStack === ruleStack2 && currentRuleStack != null) {
            //     console.error(`Finished at: ${idx}`);
            //     done = true
            //     break
            // }

            let currentLine = this.buffer.lines[idx].contents
            const r = this.grammar.tokenizeLine2(currentLine, ruleStack2)
            const tokens = Array.prototype.slice.call(r.tokens)

            resultsToSend.push({
                line: idx,
                tokens,
                ruleStack2: r.ruleStack,
            })

            ruleStack2 = r.ruleStack
            idx = idx + 1
        }

        this.store.commitUpdates(this.buffer.id, this.buffer.version, resultsToSend)

        let ret: Job.Job[] = []
        if (idx < this.buffer.lines.length && !done) {
            // console.error(`Queuing new job: ${idx}`);
            ret = [new TokenizationJob(this.buffer, idx, 500, this.grammar, this.store, 1000)]
        } else {
            // console.error(`Finished, no need to queue another job: ${idx}`);
        }

        return ret
    }
}
