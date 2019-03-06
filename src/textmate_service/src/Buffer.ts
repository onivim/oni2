/*
 * Buffer.ts
 *
 * Data type for manipulating and update buffers
 */

import * as vsctm from "vscode-textmate"

interface BufferLine {
    version: number,
    contents: string,
    ruleStack1: vsctm.StackElement,
    ruleStack2: vsctm.StackElement,
}

interface Buffer {
    lines: BufferLine[]
    version: number,
};

namespace Buffer {
    let _toBufferUpdateLine = (version: number) => (l: string) => {
            return {
                contents: l,
                version,
                ruleStack1: null as any,
                ruleStack2: null as any,
            };
        
    };

    let create = (lines: string[], version: number) => {
        let bufferLines: BufferLine[] = lines.map(_toBufferUpdateLine(version))

        let ret: Buffer = {
            lines: bufferLines,
            version,
        }

        return ret;
    };

    let update = (buf: Buffer, bufferUpdate: IBufferUpdate) => {
        if (bufferUpdate.startLine === 0 && bufferUpdate.endLine === -1) {
            return create(bufferUpdate.lines, bufferUpdate.version);
        } else {
            let beforeLines = buf.lines.slice(0, bufferUpdate.startLine);
            let afterLines = buf.lines.slice(bufferUpdate.endLine, buf.lines.length);
            let newLines = ([] as BufferLine[]).concat(beforeLines, bufferUpdate.lines.map(_toBufferUpdateLine(bufferUpdate.version)), afterLines);

            return {
                ...buf,
                version: bufferUpdate.version,
                lines: newLines,
            };
        }
    };
}
