open EditorCoreTypes;

type t = {
    startLine: LineNumber.t,
    endLine: LineNumber.t,
    startByte: ByteIndex.t,
    endByte: ByteIndex.t,
    startCharacter: CharacterIndex.t,
    endCharacter: CharacterIndex.t,
    text: array(string),
};

let diff = (originalBuffer: Buffer.t, newBuffer: Buffer.t) => {
    ignore(originalBuffer);
    ignore(newBuffer);
    
    {
        startLine: LineNumber.zero,
        endLine: LineNumber.zero,
        startByte: ByteIndex.zero,
        endByte: ByteIndex.zero,
        startCharacter: CharacterIndex.zero,
        endCharacter: CharacterIndex.zero,
        text: [||]
    }
};
