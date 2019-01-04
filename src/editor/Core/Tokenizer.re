/*
 * Tokenizer.re
 */

open Types;

type t = {
    text: string,
    startPosition: Position.t,
    endPosition: Position.t,
};

let tokenize: string => list(t) = (_s) => {
    [];   
};
