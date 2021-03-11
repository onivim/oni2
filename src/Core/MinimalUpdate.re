open EditorCoreTypes;

type update = {
    startLine: LineNumber.t,
    endLine: LineNumber.t,
    lines: array(string),
};

type t = list(update);

module Internal = {
    let compute = (~original, ~updated) => {
        let _originalLineCount = Array.length(original);
        let _updatedLineCount = Array.length(updated);

        let (deletes, adds) = Diff.f(original, updated);

        let isAdded = i => {
            i < Array.length(adds) && adds[i]
        };

        let isDeleted = i => {
            i < Array.length(deletes) && deletes[i]
        };

        // TODO: Fix recursion
        // Maybe track index for original and udpated separately?
        let rec loop = (acc, shift, idx) => {

           switch (isDeleted(idx), isAdded(idx + shift)) {
           | (true, true) => 
           let lineIdx = LineNumber.ofZeroBased(idx);
           loop([{
            startLine: lineIdx,
            endLine: LineNumber.(lineIdx + 1),
            lines: [|original[idx]|]
           }, ...acc], shift, idx + 1)
           | _ => loop(acc, shift, idx + 1)
           }
        };
        
        loop([], 0, 0);
    }

    let%test_module "compute" = (module {
        let%test "empty arrays result in no updates" = {
            let original = [||];
            let updated = [||];
            compute(~original, ~updated) == [];
        };

        let%test "modification gives an edit" = {
            let original = [|"a"|];
            let updated = [|"b"|];
            compute(~original, ~updated) == [{
                startLine: LineNumber.zero,
                endLine: LineNumber.(zero + 1),
                lines: [||]
            }];
        };
    });
}

let compute = (~original, ~updated) => {

    let originalLines = Buffer.getLines(original);
    let updatedLines = Buffer.getLines(updated);

    Internal.compute(~original=originalLines, ~updated=updatedLines);
};

