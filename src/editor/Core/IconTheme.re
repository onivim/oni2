/*
 * IconTheme.re
 *
 * Typing / schema for icon themes
 */

open Revery;

module FontSource = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    path: string,
    /* format: string, */
  };
};

[@deriving (show({with_path: false}), yojson({strict: false}))]
type fontSources = list(FontSource.t);

module Font = {
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

module IconDefinition = {
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
  };

  let parseId: string => int =
    v => {
      let len = String.length(v);
      let sub = String.sub(v, 1, len - 1);
      "0x" ++ sub |> int_of_string;
    };

  let of_raw: raw => t =
    raw => {
      {
        id: raw.id,
        fontCharacter: parseId(raw.fontCharacter),
        fontColor: Color.hex(raw.fontColor),
      };
    };
};

[@deriving (show({with_path: false}), yojson({strict: false}))]
type iconDefinitions = list(IconDefinition.raw);

type t = {
  fonts: list(Font.t),
  iconDefinitions: StringMap.t(IconDefinition.t),
  file: string,
  fileExtensions: StringMap.t(string),
  fileNames: StringMap.t(string),
  languageIds: StringMap.t(string),
  /* TODO: Light mode */
};

let getIconForFile: (t, string, string) => option(IconDefinition.t) =
  (iconTheme: t, fileName: string, languageId: string) => {
    let id =
      switch (StringMap.find_opt(fileName, iconTheme.fileNames)) {
      | Some(v) => v
      | None =>
        switch (
          StringMap.find_opt(
            Rench.Path.extname(fileName),
            iconTheme.fileExtensions,
          )
        ) {
        | Some(v) => v
        | None =>
          switch (StringMap.find_opt(languageId, iconTheme.languageIds)) {
          | Some(v) => v
          | None => iconTheme.file
          }
        }
      };

    StringMap.find_opt(id, iconTheme.iconDefinitions);
  };

let create = () => {
  fonts: [],
  iconDefinitions: StringMap.empty,
  file: "",
  fileExtensions: StringMap.empty,
  fileNames: StringMap.empty,
  languageIds: StringMap.empty,
};

let assocToStringMap = json => {
  switch (json) {
  | `Assoc(v) =>
    List.fold_left(
      (prev, curr) =>
        switch (curr) {
        | (s, `String(v)) => StringMap.add(s, v, prev)
        | _ => prev
        },
      StringMap.empty,
      v,
    )
  | _ => StringMap.empty
  };
};

let getOrEmpty = (v: result(list('a), 'b)) => {
  switch (v) {
  | Ok(v) => v
  | Error(_) => []
  };
};

let ofJson = (json: Yojson.Safe.json) => {
  switch (json) {
  | `Assoc([
      ("fonts", fontsJson),
      ("iconDefinitions", iconsJson),
      ("file", `String(file)),
      ("fileExtensions", extensionsJson),
      ("fileNames", fileNamesJson),
      ("languageIds", languageIdsJson),
      ..._,
    ]) =>
    let fonts = fonts_of_yojson(fontsJson) |> getOrEmpty;

    let toIconMap: list(IconDefinition.t) => StringMap.t(IconDefinition.t) = (
      icons => {
        List.fold_left(
          (prev, curr: IconDefinition.t) =>
            StringMap.add(curr.id, curr, prev),
          StringMap.empty,
          icons,
        );
      }
    );

    let iconDefinitions =
      iconDefinitions_of_yojson(iconsJson)
      |> getOrEmpty
      |> List.map(v => IconDefinition.of_raw(v))
      |> toIconMap;

    let fileExtensions = assocToStringMap(extensionsJson);
    let fileNames = assocToStringMap(fileNamesJson);
    let languageIds = assocToStringMap(languageIdsJson);

    Some({
      fonts,
      iconDefinitions,
      file,
      fileExtensions,
      fileNames,
      languageIds,
    });
  | _ => None
  };
};
