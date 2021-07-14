open Oni_Core;

type model = unit;
let initial = ();

[@deriving show]
type command =
  | JoinDevTalk
  | JoinDiscord
  | JoinTwitter
  | ReportBug
  | Website
  | Documentation;

[@deriving show]
type msg =
  | Command(command);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update = (msg, model) =>
  switch (msg) {
  | Command(JoinDevTalk) => (
      model,
      Effect(
        Service_OS.Effect.openURL("https://forum.devtalk.com/tag/onivim"),
      ),
    )

  | Command(JoinDiscord) => (
      model,
      Effect(Service_OS.Effect.openURL("https://discord.gg/7maEAxV")),
    )

  | Command(JoinTwitter) => (
      model,
      Effect(Service_OS.Effect.openURL("https://twitter.com/oni_vim")),
    )

  | Command(ReportBug) => (
      model,
      Effect(
        Service_OS.Effect.openURL(
          "https://github.com/onivim/oni2/issues/new",
        ),
      ),
    )

  | Command(Website) => (
      model,
      Effect(Service_OS.Effect.openURL("https://v2.onivim.io")),
    )

  | Command(Documentation) => (
      model,
      Effect(
        Service_OS.Effect.openURL(
          "https://onivim.github.io/docs/getting-started/modal-editing-101",
        ),
      ),
    )
  };

module Commands = {
  open Feature_Commands.Schema;

  let joinTwitter = define("oni.social.joinTwitter", Command(JoinTwitter));

  let joinDiscord = define("oni.social.joinDiscord", Command(JoinDiscord));

  let joinDevTalk = define("oni.social.joinDevTalk", Command(JoinDevTalk));

  let reportBug = define("oni.help.reportBug", Command(ReportBug));

  let website = define("oni.help.website", Command(Website));

  let documentation =
    define("oni.help.documentation", Command(Documentation));

  let all = [
    joinDevTalk,
    joinTwitter,
    joinDiscord,
    reportBug,
    website,
    documentation,
  ];
};

module MenuItems = {
  open ContextMenu.Schema;
  open Commands;

  let website = command(~title="Website", website);
  let documentation = command(~title="Documentation", documentation);

  let reportBug = command(~title="Report bug", reportBug);

  let documentationGroup =
    group(
      ~order=0,
      ~parent=Feature_MenuBar.Global.help,
      [website, documentation],
    );

  let joinDevTalk = command(~title="Join Devtalk", joinDevTalk);
  let joinTwitter = command(~title="Join Twitter", joinTwitter);
  let joinDiscord = command(~title="Join Discord", joinDiscord);

  let socialGroup =
    group(
      ~order=100,
      ~parent=Feature_MenuBar.Global.help,
      [joinDevTalk, joinTwitter, joinDiscord],
    );

  let developmentGroup =
    group(~order=500, ~parent=Feature_MenuBar.Global.help, [reportBug]);
};

module Contributions = {
  let menuGroups =
    MenuItems.[socialGroup, developmentGroup, documentationGroup];

  let commands = Commands.all;
};
