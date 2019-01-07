/*
 * Tokenizer.re
 */

open Types;

type t = {
    text: string,
    startPosition: Position.t,
    endPosition: Position.t,
};

let _isWhitespace = (c) => {
    c == ' ' || c == '\t' || c == '\r' || c == '\n'
};

let _isNonWhitespace = (c) => !_isWhitespace(c);

let _moveToNextMatchingToken = (f, str, startIdx) => {
   let idx = ref(startIdx); 
   let length = String.length(str);
   let found = ref(false);

   while (idx^ < length && !(found^)) {
       let c = String.get(str, idx^);
       found := f(c)

       if (!(found^)) {
           idx := idx^ + 1;
       }
   };

   idx^
};

let _moveToNextWhitespace = _moveToNextMatchingToken(_isWhitespace);
let _moveToNextNonWhitespace = _moveToNextMatchingToken(_isNonWhitespace);

let tokenize: string => list(t) = (s) => {
    let idx = ref(0);
    let length = String.length(s);
    let tokens: ref(list(t)) = ref([]);

    while (idx^ < length) {
        let startToken = _moveToNextNonWhitespace(s, idx^);    
        let endToken = min(length, _moveToNextWhitespace(s, startToken + 1));

        if (startToken < length) {
            let length = endToken - startToken;
            let tokenText = String.sub(s, startToken, length);

            let token: t = {
                text: tokenText,
                startPosition: ZeroBasedPosition(startToken),
                endPosition: ZeroBasedPosition(endToken),
            };

            tokens := List.append([token], tokens^);
        }

        idx := endToken + 1;
    };

    List.rev(tokens^);
};
