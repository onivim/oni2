
type position = [
| `Top
| `Bottom];

module Section = {
    type t = {
        element: Revery.UI.element,
        position: position
    };
};
