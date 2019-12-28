open Oni_Core;
type callback = unit => unit;
type sneakInfo = {
  node: ref(option(Revery.UI.node)),
  callback,
};

type t = list(sneakInfo);

let _singleton = ref([]);

let register = (node: ref(option(Revery.UI.node)), callback) => {
  _singleton := [{node, callback}, ..._singleton^];
};

let unregister = (node: ref(option(Revery.UI.node))) => {
  let filter = sneakInfo => sneakInfo.node !== node;
  _singleton := List.filter(filter, _singleton^);
};

let getSneaks = () => {
  _singleton^
  |> Utility.List.filter_map(item => {
       switch (item.node^) {
       | Some(node) => Some((node, item.callback))
       | None => None
       }
     })
  |> List.map(((node, callback)) => {
       Oni_Model.Sneak.{callback, boundingBox: node#getBoundingBox()}
     });
};
