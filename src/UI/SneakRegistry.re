open Oni_Core;
type sneakInfo = {
  node: ref(option(Revery.UI.node))
};

type t = list(sneakInfo);

let _singleton = ref([]);

let register = (node: ref(option(Revery.UI.node))) => {
  _singleton := [{node: node}, ..._singleton^] 
};

let unregister = (node: ref(option(Revery.UI.node))) => {
  let filter = (sneakInfo) => sneakInfo.node !== node;
  _singleton := List.filter(filter, _singleton^);
};

let getSneaks = () => {
 /*let iter = (node) => {
    let bbox = (node)#getBoundingBox();
    prerr_endline ("NODE BBOX: " ++ Revery.Math.BoundingBox2d.toString(bbox));
 } ;*/
 let map = (node) => {
    (node)#getBoundingBox();
 };
 _singleton^
 |> List.map(({node}) => node^)
 |> Utility.List.filter_map(Utility.identity)
 |> List.map(map);
 //|> List.iter(iter);
};
