type setMenuItems = list(Actions.menuCommand) => unit;

type menuDisposeFunction = unit => unit;

type menuCreationFunction = setMenuItems => menuDisposeFunction;

type t = menuCreationFunction;
