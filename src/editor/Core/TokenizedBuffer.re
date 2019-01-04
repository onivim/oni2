/*
 * TokenizedBuffer.re
 *
 * Helper to tokenize buffer lines, for:
 * - Rendering whole words
 * - Keeping a cache for 'keyword' completion
 */

type t = {
    file: option(string),
    tokenizedLines: array(list(Tokenizer.t))
};

let ofBuffer = (buffer: Buffer.t) => {
    let f: string => list(Tokenizer.t) = Tokenizer.tokenize;
    let tokenizedLines = Array.map(f, buffer.lines);

    let ret: t = {
       tokenizedLines,
       file: buffer.file,
    };
    ret;
};
