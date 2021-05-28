open Oni_Core;

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
  module Breakpoint: {
    [@deriving show]
    type t;
  };

  [@deriving show]
  module Command: {
    type t = {
      command: string,
      title: LocalizedToken.t,
      category: option(string),
      condition: WhenExpr.t,
    };
  };

  module Debugger: {
    [@deriving show]
    type t;
  };

  module Menu: {
    [@deriving show]
    type t = Oni_Core.Menu.Schema.definition
    and item = Oni_Core.Menu.Schema.item;
  };

  module Configuration: {
    module PropertyType: {
      type t =
        | Array
        | Boolean
        | String
        | Integer
        | Number
        | Object
        | Unknown;
    };

    type t = list(property)
    and property = {
      name: string,
      default: [@opaque] Oni_Core.Json.t,
      propertyType: PropertyType.t,
    };

    let toSettings: t => Oni_Core.Config.Settings.t;
  };

  module Language: {
    [@deriving show]
    type t = {
      id: string,
      extensions: list(string),
      filenames: list(string),
      filenamePatterns: list(string),
      firstLine: option(string),
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

  module Snippet: {
    [@deriving show]
    type t = {
      language: option(string),
      path: string,
    };
  };

  module Theme: {
    [@deriving show]
    type t = {
      id: option(string),
      label: LocalizedToken.t,
      uiTheme: string,
      path: string,
    };

    let id: t => string;

    let label: t => string;
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
    breakpoints: list(Breakpoint.t),
    commands: list(Command.t),
    debuggers: list(Debugger.t),
    menus: list(Menu.t),
    languages: list(Language.t),
    grammars: list(Grammar.t),
    snippets: list(Snippet.t),
    themes: list(Theme.t),
    iconThemes: list(IconTheme.t),
    configuration: Configuration.t,
  };

  let encode: Oni_Core.Json.encoder(t);
};

module Manifest: {
  module Kind: {
    [@deriving show]
    type t =
      | Ui
      | Workspace;
  };
  [@deriving show]
  type t = {
    name: string,
    version: option(Semver.t),
    author: string,
    displayName: option(LocalizedToken.t),
    description: option(string),
    publisher: option(string),
    defaults: Yojson.Safe.t,
    main: option(string),
    icon: option(string),
    categories: list(string),
    keywords: list(string),
    engines: string,
    activationEvents: list(string),
    extensionDependencies: list(string),
    runtimeDependencies: Yojson.Safe.t,
    extensionPack: list(string),
    extensionKind: list(Kind.t),
    contributes: Contributions.t,
    enableProposedApi: bool,
  };

  let decode: Oni_Core.Json.decoder(t);

  let identifier: t => string;
  let publisher: t => string;
  let getDisplayName: t => string;
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
      rawPackageJson: Yojson.Safe.t,
    };
  };

  let load: (~category: category, string) => option(ScanResult.t);
  let scan:
    (~category: category, FpExp.t(FpExp.absolute)) => list(ScanResult.t);
};

module InitData: {
  module Identifier: {
    type t;

    let fromString: string => t;
  };

  module StaticWorkspaceData: {
    [@deriving (show, yojson({strict: false}))]
    type t = {
      id: string,
      name: string,
    };

    let global: t;
  };

  module Extension: {
    [@deriving (show, yojson({strict: false}))]
    type t;

    let ofScanResult: Scanner.ScanResult.t => t;
  };

  module Environment: {
    [@deriving (show, yojson({strict: false}))]
    type t = {
      isExtensionDevelopmentDebug: bool,
      appName: string,
      appLanguage: string,
      appRoot: Oni_Core.Uri.t,
      globalStorageHome: option(Oni_Core.Uri.t),
      workspaceStorageHome: option(Oni_Core.Uri.t),
      userHome: option(Oni_Core.Uri.t),
      webviewResourceRoot: string,
      webviewCspSource: string,
      // TODO
      /*
       appLanguage: string,
       appUriScheme: string,
       appSettingsHome: option(Uri.t),
       useHostProxy: boolean,
       */
    };

    let default: unit => t;
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
      sessionId: string,
      machineId: string,
      instanceId: string,
      msftInternal: bool,
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
    workspace: StaticWorkspaceData.t,
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
      ~workspace: StaticWorkspaceData.t=?,
      list(Extension.t)
    ) =>
    t;
};
