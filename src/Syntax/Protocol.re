/*
 * Protocol represents the communication protocol
 * between the Syntax_client and Syntax_server
 */

module ServerToClient = {
  type t =
    | EchoReply(string)
    | Log(string);
  // buffer lines
};

module ClientToServer = {
  type t =
    | Echo(string)
    | BufferEnter(int, string, array(string))
    | BufferLeave(int)
    | BufferUpdate(Oni_Core.Types.BufferUpdate.t)
    | SetLanguageInfo(LanguageInfo.t)
    | ThemeChanged(TokenTheme.t);
};
