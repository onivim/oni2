/*
 * ExtensionManifest.re
 *
 * Module to describing metadata about an extension
 */

[@deriving yojson({strict: false, exn: true})]
type t = {
    
    name: string,
    
    displayName: option(string),
};

