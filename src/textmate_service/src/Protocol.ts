/*
 * Protocol.ts
 *
 * Definition of the textmate syntax highlighting protocol
 */

import * as Protocol from "./Protocol"

export interface BufferUpdate {
    id: number
    startLine: number
    endLine: number
    lines: string[]
    version: number
}

export type BufferUpdateParams = [string, BufferUpdate]
