/*
 * IconTheme.re
 *
 * Typing / schema for icon themes
 */

open Revery;

module FontSource {
    [@deriving (show({with_path: false}), yojson({strict: false}))]
    type t  = {
        path: string,
        /* format: string, */
    }
};

[@deriving (show({with_path: false}), yojson({strict: false}))]
type fontSources = list(FontSource.t);

module Font {
    [@deriving (show({with_path: false}), yojson({strict: false}))]
    type t = {
        id: string,
        src: list(FontSource.t),
        weight: string,
        style: string,
        size: string,
    };
};

[@deriving (show({with_path: false}), yojson({strict: false}))]
type fonts = list(Font.t);

module IconDefinition {
    [@deriving (show({with_path: false}), yojson({strict: false}))]
    type raw = {
       id: string,
       fontCharacter: string,
       fontColor: string,
    };

    type t = {
        id: string,
        fontCharacter: int,
        fontColor: Color.t,
    }
};

type t = {
    fonts: list(Font.t),
    iconDefinitions: StringMap.t(IconDefinition.t),
    file: string,
    fileExtensions: StringMap.t(string),
    fileNames: StringMap.t(string),
    languageIds: StringMap.t(string),

    /* TODO: Light mode */
};

type characterInfo = {
    fontPath: string,
    fontCharacter: int,
    color: Color.t,
};

let ofJson = (_json: Yojson.Safe.json) => {
   (); 
};
