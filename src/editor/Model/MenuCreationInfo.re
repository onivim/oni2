type setMenuItems = list(MenuCommand.t) => unit;

type menuDisposeFunction = unit => unit;

type menuCreationFunction = setMenuItems => menuDisposeFunction;

type t = menuCreationFunction;
