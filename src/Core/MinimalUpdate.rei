open EditorCoreTypes;

type update = {
    startLine: LineNumber.t,
    endLine: LineNumber.t,
    lines: array(string),
};

type t = list(update);

let compute: (~original: Buffer.t, ~updated: Buffer.t) => t;
