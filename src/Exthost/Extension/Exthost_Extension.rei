module LocalizationDictionary: {
  type t;

  let initial: t;

  let of_yojson: Yojson.Safe.t => t;

  let get: (string, t) => option(string);

  let count: t => int;
};

module LocalizedToken: {
  [@deriving show]
  type t;

  let parse: string => t;

  /**
 [localize(token)] returns a new token that has been localized.
*/
  let localize: (LocalizationDictionary.t, t) => t;

  let decode: Oni_Core.Json.decoder(t);

  /*
    [to_string(token)] returns a string representation.

    If the token has been localized with [localize], and has a localization,
    the localized token will be returned.

    Otherwise, the raw token will be returned.
   */
  let toString: t => string;
};

module Contributions: {
  [@deriving show]
  module Command: {
    type t = {
      command: string,
      title: LocalizedToken.t,
      category: option(string),
      condition: WhenExpr.t,
    };
  };

  module Menu: {
    [@deriving show]
    type t = Oni_Core.Menu.Schema.definition
    and item = Oni_Core.Menu.Schema.item;
  };

  module Configuration: {
    type t = list(property)
    and property = {
      name: string,
      default: [@opaque] Oni_Core.Json.t,
    };

    let toSettings: t => Oni_Core.Config.Settings.t;
  };

  module Language: {
    [@deriving show]
    type t = {
      id: string,
      extensions: list(string),
      aliases: list(string),
      configuration: option(string),
    };
  };

  module Grammar: {
    [@deriving show]
    type t = {
      language: option(string),
      scopeName: string,
      path: string,
      treeSitterPath: option(string),
    };
  };

  module Theme: {
    [@deriving show]
    type t = {
      label: string,
      uiTheme: string,
      path: string,
    };
  };

  module IconTheme: {
    [@deriving show]
    type t = {
      id: string,
      label: string,
      path: string,
    };
  };

  [@deriving show]
  type t = {
    commands: list(Command.t),
    menus: list(Menu.t),
    languages: list(Language.t),
    grammars: list(Grammar.t),
    themes: list(Theme.t),
    iconThemes: list(IconTheme.t),
    configuration: Configuration.t,
  };

  let encode: Oni_Core.Json.encoder(t);
};

module Manifest: {
  [@deriving show]
  type t = {
    name: string,
    version: string,
    author: string,
    displayName: option(LocalizedToken.t),
    description: option(string),
    // publisher: option(string),
    main: option(string),
    icon: option(string),
    categories: list(string),
    keywords: list(string),
    engines: string,
    activationEvents: list(string),
    extensionDependencies: list(string),
    extensionPack: list(string),
    extensionKind: kind,
    // TODO: Bring back
    contributes: Contributions.t,
    enableProposedApi: bool,
  }

  and kind =
    | Ui
    | Workspace;

  let decode: Oni_Core.Json.decoder(t);

  let getDisplayName: t => string;

  module Encode: {let kind: Oni_Core.Json.encoder(kind);};
};

module Scanner: {
  type category =
    | Default
    | Bundled
    | User
    | Development;

  module ScanResult: {
    type t = {
      category,
      manifest: Manifest.t,
      path: string,
    };
  };

  let load:
    (~prefix: option(string)=?, ~category: category, string) =>
    option(ScanResult.t);
  let scan:
    (~prefix: option(string)=?, ~category: category, string) =>
    list(ScanResult.t);
};

module InitData: {
  module Extension: {
    [@deriving (show, yojson({strict: false}))]
    type t = {
      identifier: string,
      extensionLocation: Oni_Core.Uri.t,
      name: string,
      main: option(string),
      version: string,
      engines: string,
      activationEvents: list(string),
      extensionDependencies: list(string),
      extensionKind: string,
      enableProposedApi: bool,
    };

    let ofManifestAndPath: (Manifest.t, string) => t;
  };

  module Environment: {
    [@deriving (show, yojson({strict: false}))]
    type t = {
      isExtensionDevelopmentDebug: bool,
      appName: string,
      appLanguage: string,
      // TODO
      /*
       appRoot: option(Types.Uri.t),
       appLanguage: string,
       appUriScheme: string,
       appSettingsHome: option(Uri.t),
       globalStorageHome: Uri.t,
       userHome: Uri.t,
       webviewResourceRoot: string,
       webviewCspSource: string,
       useHostProxy: boolean,
       */
    };

    let default: t;
  };

  module Remote: {
    [@deriving (show, yojson({strict: false}))]
    type t = {
      isRemote: bool,
      // TODO:
      // authority: string,
    };

    let default: t;
  };

  [@deriving (show, yojson({strict: false}))]
  module TelemetryInfo: {
    [@deriving (show, yojson({strict: false}))]
    type t = {
      sessionId: int,
      machineId: int,
      instanceId: int,
    };

    let default: t;
  };

  [@deriving (show, yojson({strict: false}))]
  type t = {
    version: string,
    parentPid: int,
    extensions: list(Extension.t),
    resolvedExtensions: list(unit),
    hostExtensions: list(unit),
    environment: Environment.t,
    logLevel: int,
    logsLocation: Oni_Core.Uri.t,
    logFile: Oni_Core.Uri.t,
    autoStart: bool,
    remote: Remote.t,
    telemetryInfo: TelemetryInfo.t,
  };

  let create:
    (
      ~version: string,
      ~parentPid: int,
      ~logsLocation: Oni_Core.Uri.t,
      ~logFile: Oni_Core.Uri.t,
      ~environment: Environment.t=?,
      ~logLevel: int=?,
      ~autoStart: bool=?,
      ~remote: Remote.t=?,
      ~telemetryInfo: TelemetryInfo.t=?,
      list(Extension.t)
    ) =>
    t;
};
