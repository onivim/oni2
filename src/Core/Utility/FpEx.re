let explode = (fp: Fp.t('a)) => {
  let rec loop = (acc, currentPath) => {
    switch (Fp.baseName(currentPath)) {
    | None => acc
    | Some(basename) => loop([basename, ...acc], Fp.dirName(currentPath))
    };
  };

  loop([], fp);
};
