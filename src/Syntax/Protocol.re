/*
 * Protocol represents the communication protocol
 * between the Syntax_client and Syntax_server
 */

module Ext = Oni_Extensions;

module ServerToClient = {
  [@deriving show({with_path: false})]
  type t =
    | EchoReply(string)
    | Log(string);
  // buffer lines
};

module ClientToServer = {
  [@deriving show({with_path: false})]
  type t =
    | Echo(string)
    | Initialize([@opaque] Ext.LanguageInfo.t)
    | BufferEnter(int, string, [@opaque] array(string))
    | BufferLeave(int)
    | BufferUpdate([@opaque] Oni_Core.BufferUpdate.t)
    | ThemeChanged([@opaque] TokenTheme.t);
};
