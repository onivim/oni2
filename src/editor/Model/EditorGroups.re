/*
 * EditorGroups.re
 *
 * Managing an aggregate of EditorGroups
 */

type t = IntMap.t(EditorGroup.t);

let create = () => {
   let defaultGroup = EditorGroup.create(); 
   IntMap.add(defaultGroup.id, defaultGroup, IntMap.empty);
};

let reduce = (v: t, action: Actions.t) => {
    IntMap.fold(
        (key, value, prev) => 
        IntMap.add(key, EditorGroup..reduce(value, action), prev),
        v,
        IntMap.empty
    );
}
