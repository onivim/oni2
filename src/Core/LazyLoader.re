/*
 * LazyLoader.re
 */

type loadFunction('a) = string => result('a, string);

type t('a) = {
  loader: loadFunction('a),
  cache: Hashtbl.t(string, result('a, string)),
};

let create = loader => {
  let cache = Hashtbl.create(16);
  {loader, cache};
};

let get = ({cache, loader}, key) => {
  switch (Hashtbl.find_opt(cache, key)) {
  | Some(cachedResult) => cachedResult
  | None =>
    let loadedResult = loader(key);
    Hashtbl.add(cache, key, loadedResult);
    loadedResult;
  };
};

let fail = create((_: string) => Error("Fail"));
let success = v => create((_: string) => Ok(v));
