/*
 * Buffer.ts
 *
 * Data type for manipulating and update buffers
 */

import * as vsctm from "vscode-textmate"
import * as Protocol from "./Protocol"

export interface BufferLine {
    version: number,
    contents: string,
};

export interface Buffer {
    lines: BufferLine[]
    version: number,
};

let _toBufferUpdateLine = (version: number) => (l: string) => {
        return {
            contents: l,
            version,
        };
};

export let create = (lines: string[], version: number) => {
    let bufferLines: BufferLine[] = lines.map(_toBufferUpdateLine(version))

    let ret: Buffer = {
        lines: bufferLines,
        version,
    }

    return ret;
};

export let update = (buf: Buffer, bufferUpdate: Protocol.BufferUpdate) => {
    if (bufferUpdate.startLine == 0 && bufferUpdate.endLine == -1) {
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

export let print = (buf: Buffer) => {
    buf.lines.forEach((l, i) => console.error(`Line ${i}: ${l.contents}`));
};
