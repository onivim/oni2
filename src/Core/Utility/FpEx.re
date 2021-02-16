
let explode = (fp: Fp.t('a)) => {
    let rec loop = (acc, currentPath) => {
        switch (Fp.baseName(currentPath)) {
        | None => acc
        | Some(basename) => loop([basename, ...acc], Fp.dirName(currentPath))
        }
    };

    prerr_endline ("Exploding: " ++ Fp.toDebugString(fp));
    let ret = loop([], fp);
    // |> List.rev;

    ret
    |> List.iter(str => prerr_endline (" - " ++ str));

    ret;
};
