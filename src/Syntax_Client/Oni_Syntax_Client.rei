/*
 Oni_Syntax_Client.rei

 A client for Oni_Syntax_Server - a separate process for managing syntax highlights
 */

open EditorCoreTypes;

module Core = Oni_Core;
module Ext = Oni_Extensions;

open Oni_Syntax;
module Protocol = Oni_Syntax.Protocol;
module ServerToClient = Protocol.ServerToClient;

type t;

type connectedCallback = unit => unit;
type closeCallback = int => unit;
type scheduler = (unit => unit) => unit;
type highlightsCallback = (list(Protocol.TokenUpdate.t)) => unit;

let start:
    (
      ~onConnected:connectedCallback=?,
      ~onClose:closeCallback=?,
      ~scheduler:scheduler,
      ~onHighlights:highlightsCallback,
      Ext.LanguageInfo.t,
      Core.Setup.t
    ) => t;

let notifyBufferEnter: (t, int, string) => unit;
let notifyBufferLeave: (t, int) => unit;
let notifyThemeChanged: (t, TokenTheme.t) => unit;
let notifyConfigurationChanged: (t, Core.Configuration.t) => unit;
let notifyBufferUpdate: (t, Core.BufferUpdate.t, array(string), string) => unit;

let notifyVisibilityChanged: (t, list((int, list(Range.t)))) => unit;
let close: t => unit;

