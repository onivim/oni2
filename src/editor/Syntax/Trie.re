/*
Trie.re

Simple Trie implementation to support scope selection
*/

module StringMap = Oni_Core.StringMap;

    type t('a) = {
        v: option('a),
        prefix: string,
        children: StringMap.t(t('a)),
    };

    let empty: t('a) = {
        v: None,
        prefix: "",
        children: StringMap.empty
    };

    let rec update = (path: list(string), f: option('a) => option('a), tree: t('a)) => {
        switch (path) {
        | [hd, ...tail] => {

            let recurse = (childTree: option(t('a))) => {
                switch (childTree) {
                | None => Some(update(tail, f, { ...empty, prefix: hd }))
                | Some(v) => Some(update(tail, f, v));
                }
            };
            
            let children = StringMap.update(hd, recurse, tree.children);
            {
                ...tree,
                children,
            }
        }
        | [] => {
            let newVal = f(tree.v);
            {
                ...tree,
                v: newVal,
            }
        }
        };
    };

    let matches = (tree: t('a), path: list(string)) => {
        let rec f = (tree: t('a), path: list(string), curr: list((string, option('a)))) => {
            switch (path) {
            | [hd, ...tail] => {
                switch (StringMap.find_opt(hd, tree.children)) {
                | Some(v) => f(v, tail, [(hd, v.v), ...curr])
                | None => curr
                }
            }
            | [] => curr
            }
        }

        f(tree, path, []);
    }
