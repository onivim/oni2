/*
 * ExtensionHostInitData.re
 *
 * Module documenting the initialization data required for spinning up
 * and activating the extension host
 */

module ExtensionInfo {
    type t;
};

module Workspace {
   type t; 
};

module Environment {
    type t = {
        globalStorageHomePath: string,
    };
};

type t = {
    extensions: list(ExtensionInfo.t),
    parentPid: int,
    environment: Environment.t,
    logsLocationPath: string,
    autoStart: bool,
};
