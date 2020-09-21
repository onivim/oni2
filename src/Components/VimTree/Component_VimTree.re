open Oni_Core;

[@deriving show]
type msg = 
| List(Component_VimList.msg);

type nodeOrLeaf('node, 'leaf) =
| Node({ expanded: bool, indentation: int, data: 'node })
| Leaf({ indentation: int, data: 'leaf });

[@deriving show]
type model('node, 'leaf) = {
    treeAsList: Component_VimList.model(Tree.listView('node, 'leaf)),
};

let create = (~rowHeight) => {
    treeAsList: Component_VimList.create(~rowHeight),
};

// UPDATE

type outmsg('node, 'leaf) =
  | Nothing
  | Expanded('node)
  | Collapsed('node)
  | Selected('leaf);

let update = (msg, model) => {
    switch (msg) {
    | List(listMsg) => 
        let (treeAsList, _outmsg) = Component_VimList.update(listMsg, model.treeAsList);
        
        ({treeAsList: treeAsList}, Nothing)
    }
}

let set = (trees: list(Tree.t('node, 'leaf)), model: model('node, 'leaf)) => {
    let treeAsList = trees
    |> List.map(Tree.toList)
    |> List.flatten
    |> Array.of_list;

    {
    treeAsList: Component_VimList.set(treeAsList, model.treeAsList)
    };
}

module Contributions = {
    let commands = Component_VimList.Contributions.commands
          |> List.map(Oni_Core.Command.map(msg => List(msg)))
    let contextKeys = Component_VimList.Contributions.contextKeys;
};

module View = {
    let make = (
        ~theme,
        ~model,
        ~dispatch,
        ~render,
        unit,
    ) => {
        Revery.UI.React.empty
    }
}

