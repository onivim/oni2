/*
 * Protocol represents the communication protocol
 * between the Syntax_client and Syntax_server
 */

module Ext = Oni_Extensions;

module ServerToClient = {
  type t =
    | EchoReply(string)
    | Log(string);
  // buffer lines
};

module ClientToServer = {
  type t =
    | Echo(string)
    | Initialize(Ext.LanguageInfo.t)
    | BufferEnter(int, string, array(string))
    | BufferLeave(int)
    | BufferUpdate(Oni_Core.BufferUpdate.t)
    | ThemeChanged(TokenTheme.t);
};
