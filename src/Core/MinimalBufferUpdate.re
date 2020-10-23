open EditorCoreTypes;

[@deriving show]
type t = {
    startLine: LineNumber.t,
    endLine: LineNumber.t,
    startByte: ByteIndex.t,
    endByte: ByteIndex.t,
    startCharacter: CharacterIndex.t,
    endCharacter: CharacterIndex.t,
    text: array(string),
};

module AbstractBuffer = {
    type t = {
        numberOfLines: int,
        getLine: (int) => string
    };

    let ofArray = (lines) => {
        numberOfLines: Array.length(lines),
        getLine: idx => lines[idx]
    };

    let slice = (~start, ~length, buf) => {
      let len = buf.numberOfLines;
      if (start >= len) {
        [||];
      } else {
        let start = max(start, 0);
        let len = min(start + length, len) - start;
        if (len <= 0) {
          [||];
        } else {
          Array.init(len, (idx) => {
            buf.getLine(start + idx);
          });
        };
      };
    }
}

module Internal = {
    let findLastDeltaInLine = (oldLine, newLine) => {
        let rec loop = (oldByte, newByte, oldCharacter, newCharacter) => {
           if (oldByte < 0 && newByte < 0) {
             // Got to the beginning, with no delta
             None
           }  else if(oldByte < 0) {
             Some((0, 0))
           } else if (newByte < 0) {
             Some((oldByte, oldCharacter))
           } else {
                // We're not at the boundary - let's take a look at the characters in both places
                let (oldUchar, oldPrevByte) = Zed_utf8.unsafe_extract_prev(oldLine, oldByte); 
                let (newUchar, newPrevByte) = Zed_utf8.unsafe_extract_prev(newLine, newByte);

                if (Uchar.equal(oldUchar, newUchar)) {
                    // Same character, keep looking...
                    loop(oldPrevByte, oldCharacter - 1, newPrevByte, newCharacter - 1)
                } else {
                    // There's a diff!
                   Some((newByte + 1, newCharacter + 1))
                }
           }
        };

        loop(String.length(oldLine), String.length(newLine), Zed_utf8.length(oldLine),
        Zed_utf8.length(newLine));
    };

    let findFirstDeltaInLine = (oldLine, newLine) => {
        let rec loop = (byte, character) => {
            if (byte >= String.length(oldLine) && byte >= String.length(newLine)) {
                // Got to the end for both of these lines
                None
            } else if (byte >= String.length(oldLine)) {
               //  We've passed the old line... so there must be new stuff after
               Some((byte - 1, character - 1))
            } else if(byte >= String.length(newLine)) {
               Some((byte, character))
            } else {
                // We're not at the boundary - let's take a look at the characters in both places
                let (oldUchar, oldNextByte) = Zed_utf8.unsafe_extract_next(oldLine, byte); 
                let (newUchar, newNextByte) = Zed_utf8.unsafe_extract_next(newLine, byte);

                if (Uchar.equal(oldUchar, newUchar) && oldNextByte == newNextByte) {
                    // Same character, keep looking...
                    loop(oldNextByte, character + 1)
                } else {
                    // There's a diff!
                   Some((byte, character))
                }
            }
        };
        loop(0, 0);
    };


    let diff = (originalBuffer: AbstractBuffer.t, newBuffer: AbstractBuffer.t) => {
        ignore(originalBuffer);
        ignore(newBuffer);


        let rec loop = (startDiff, endDiff, forwardLineCursor, oldBackwardLineCursor, newBackwardLineCursor) => {

            prerr_endline(Printf.sprintf(
                "Iterate - forward: %d oldBack: %d, newBack: %d",
                forwardLineCursor,
                oldBackwardLineCursor,
                newBackwardLineCursor,
            ))
            
            ignore(startDiff);
            ignore(endDiff);
            if (forwardLineCursor > oldBackwardLineCursor && forwardLineCursor > newBackwardLineCursor) {
                // We've traversed far enough to know there is no diff!
                switch ((startDiff, endDiff)) {
                | (None, None) => None
                | (Some((byte0, char0)), Some((byte1, char1))) =>
                    let startLineIdx = forwardLineCursor - 1;
                    let endLineIdx = oldBackwardLineCursor + 1;
                    let endByteIdx = byte1;
                    let endCharIdx = char1;
                    if (startLineIdx == endLineIdx) {
                        let origLine = originalBuffer.getLine(startLineIdx);
                        let newLine = newBuffer.getLine(startLineIdx);

                        if (String.length(origLine) > String.length(newLine)) {
                            // Delete
                            Some({
                                startLine: LineNumber.ofZeroBased(startLineIdx),
                                endLine: LineNumber.ofZeroBased(endLineIdx),
                                startByte: ByteIndex.ofInt(byte0),
                                endByte: ByteIndex.ofInt(endByteIdx),
                                startCharacter: CharacterIndex.ofInt(char0),
                                endCharacter: CharacterIndex.ofInt(endCharIdx), 
                                // TODO
                                text: [||],
                            })
                        } else {
                            Some({
                                startLine: LineNumber.ofZeroBased(startLineIdx),
                                endLine: LineNumber.ofZeroBased(endLineIdx),
                                startByte: ByteIndex.ofInt(byte0),
                                endByte: ByteIndex.ofInt(byte0),
                                startCharacter: CharacterIndex.ofInt(char0),
                                endCharacter: CharacterIndex.ofInt(char0), 
                                text: [|String.sub(newLine, byte0, endByteIdx-byte0)|],
                            })
                        }
                    } else {
                        Some({
                            startLine: LineNumber.ofZeroBased(startLineIdx),
                            endLine: LineNumber.ofZeroBased(endLineIdx),
                            startByte: ByteIndex.ofInt(byte0),
                            endByte: ByteIndex.ofInt(endByteIdx),
                            startCharacter: CharacterIndex.ofInt(char0),
                            endCharacter: CharacterIndex.ofInt(endCharIdx), 
                            // TODO
                            text: [||],
                        })
                    };
                    // Can we hit this case?
                | _ => None
                }
            } else {
                // Hit end of current buffer... everything else is diff
                if (forwardLineCursor >= originalBuffer.numberOfLines) {
                    Some({
                        startLine: LineNumber.ofZeroBased(originalBuffer.numberOfLines),
                        endLine: LineNumber.ofZeroBased(originalBuffer.numberOfLines),
                        startByte: ByteIndex.zero,
                        endByte: ByteIndex.zero,
                        startCharacter: CharacterIndex.zero,
                        endCharacter: CharacterIndex.zero,
                        text: AbstractBuffer.slice(
                            ~start=forwardLineCursor,
                            ~length=newBuffer.numberOfLines - forwardLineCursor,
                            newBuffer,
                        ),
                    })
                } else if (forwardLineCursor >= newBuffer.numberOfLines) {

                    // Hit end of new buffer
                    Some({
                        startLine: LineNumber.ofZeroBased(forwardLineCursor),
                        endLine: LineNumber.ofZeroBased(forwardLineCursor),
                        startByte: ByteIndex.zero,
                        endByte: ByteIndex.zero,
                        startCharacter: CharacterIndex.zero,
                        endCharacter: CharacterIndex.zero,
                        text: [||]
                    })
                } else {
                    // Traverse again
                    let (startDiff', forwardLineCursor') = switch (startDiff) {
                    | Some(_) as d => (d, forwardLineCursor)
                    | None =>
                        let oldLine = originalBuffer.getLine(forwardLineCursor);
                        let newLine = newBuffer.getLine(forwardLineCursor);
                        (findFirstDeltaInLine(oldLine, newLine), forwardLineCursor + 1)
                    };

                    let (endDiff', oldBackwardLineCursor', newBackwardLineCursor') = switch(endDiff) {
                        | Some(_) as diff => (diff, oldBackwardLineCursor, newBackwardLineCursor)

                        | None =>
                            let oldLine = originalBuffer.getLine(oldBackwardLineCursor);
                            let newLine = newBuffer.getLine(newBackwardLineCursor);

                        (findLastDeltaInLine(oldLine, newLine), oldBackwardLineCursor - 1, newBackwardLineCursor - 1);
                    }

                    loop (startDiff', endDiff', forwardLineCursor', 
                    oldBackwardLineCursor', newBackwardLineCursor');
                }
            }
        };

        loop(None, None, 0, originalBuffer.numberOfLines - 1, newBuffer.numberOfLines - 1);

//        let oldEndPosition = BytePosition.{
//            line: LineNumber.ofZeroBased(oldBuffer.numberOfLines),
//            byte: String.length(oldBuffer.getLine(oldBuffer.numberOfLines - 1))
//        };
//        let newEndPosition = BytePosition.{
//            line: LineNumber.ofZeroBased(newBuffer.numberOfLines),
//            byte: String.length(newBuffer.getLine(newBuffer.numberOfLines - 1))
//        };

        
//        None
//        Some({
//            startLine: LineNumber.zero,
//            endLine: LineNumber.zero,
//            startByte: ByteIndex.zero,
//            endByte: ByteIndex.zero,
//            startCharacter: CharacterIndex.zero,
//            endCharacter: CharacterIndex.zero,
//            text: [||]
//        })
    };
};


let%test_module "diff" =
  (module
   {
            open Internal;
            let initial = AbstractBuffer.ofArray([|
            "abc",
            "def",
            "ghi",
            |]);
//
            let insertZ = AbstractBuffer.ofArray([|
            "abc",
            "dezf",
            "ghi",
            |]);
            let insertXYZ = AbstractBuffer.ofArray([|
            "abc",
            "dexyzf",
            "ghi",
            |]);

            let line1 = LineNumber.ofZeroBased(1);
            let byte2 = ByteIndex.ofInt(2);
            let char2 = CharacterIndex.ofInt(2);
            let byte3 = ByteIndex.ofInt(3);
            let char3 = CharacterIndex.ofInt(3);
//            let byte5 = ByteIndex.ofInt(5);
//            let char5 = CharacterIndex.ofInt(5);
//
        let%test "no diff" = {
            diff(initial, initial)  == None;
        };

        let showHelper = maybeDiff => switch(maybeDiff) {
        | None => prerr_endline ("NONE");
        | Some(v) => prerr_endline ("Some: " ++ show(v));
        }

        let%test "single-line: insert single character" = {
            let ret = diff(initial, insertZ);
            ret |> showHelper;
                ret == Some({
                startLine: line1,
                endLine: line1,
                startByte: byte2,
                endByte: byte2,
                startCharacter: char2,
                endCharacter: char2,
                text: [|"z"|]
            });
        }
        let%test "single-line: remove single character" = {
            let ret = diff(insertZ, initial);
            ret |> showHelper;
                ret == Some({
                startLine: line1,
                startByte: byte2,
                startCharacter: char2,
                endLine: line1,
                endByte: byte3,
                endCharacter: char3,
                text: [||]
            });
        }
        let%test "single-line: insert multiple characters" = {
            let ret = diff(initial, insertXYZ);
            ret |> showHelper;
                ret == Some({
                startLine: line1,
                endLine: line1,
                startByte: byte2,
                endByte: byte2,
                startCharacter: char2,
                endCharacter: char2,
                text: [|"xyz"|]
            });
        }
   });
