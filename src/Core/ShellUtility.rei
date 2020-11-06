// Get the default shell command for the user
let getDefaultShell: unit => string;

// Get the value of the $PATH environment variable from the login shell
let getPathFromShell: unit => string;

// Get the path from the current process environment
let getPathFromEnvironment: unit => string;

// Get a key-value pair representing the default shell environment
let getDefaultShellEnvironment: unit => Kernel.StringMap.t(string);

let fixOSXPath: unit => unit;
