type tree('node, 'leaf) =
| Leaf('leaf)
| Node({
    expanded: bool,
    data: 'node,
    children: list(tree('node, 'leaf))
});

type treeView('node, 'leaf) =
| ViewLeaf({ indentationLevel: int, data: 'leaf })
| ViewNode({
    expanded: bool,
    indentationLevel: int,
    data: 'node,
});

let toList: tree('node, 'leaf') => list(treeView('node, 'leaf)) = tree => {
    let rec loop = (acc,node, indentationLevel) => {
        switch (node) {
        | Leaf(data) => [ViewLeaf({indentationLevel, data})]
        | Node({expanded, data, children}) => 
         let currentRoot = ViewNode({
            expanded,
            data,
            indentationLevel
        });
        let currentAcc = [currentRoot, ...acc];
        List.fold_left((acc, curr) => {
           loop(acc, curr, indentationLevel + 1); 
        }, currentAcc, children);
        }
    };

    loop([], tree, 0);
};
