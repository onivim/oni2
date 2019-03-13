/*
 * ExtensionManifest.re
 *
 * Module to describing metadata about an extension
 */

module ExtensionKind {
    [@deriving (show, yojson({strict: false, exn: true}))]
    type t = 
    | Ui [@name "ui"]
    | Workspace [@name "workspace"];
}

[@deriving (show, yojson({strict: false, exn: true}))]
type t = {
    
    name: string,
    displayName: option(string),
    description: option(string),
    main: option(string),
    icon: option(string),
    categories: list(string),
    keywords: list(string),
    activationEvents: list(string),
    extensionDependencies: list(string),
    extensionPack: list(string),
    extensionKind: ExtensionKind.t,
};

