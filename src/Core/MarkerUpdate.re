open EditorCoreTypes;

type t = unit;

let create = (
    ~update,
    ~original,
    ~updated,
) => ();

let apply = (
    ~shiftLines,
    ~shiftCharacters,
    markerUpdate,
    target
) => target;
