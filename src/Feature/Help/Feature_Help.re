open Oni_Core;

type model = unit;
let initial = ();

[@deriving show]
type command =
  | JoinDevTalk
  | JoinDiscord
  | JoinTwitter;

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
  };

module Commands = {
  open Feature_Commands.Schema;

  let joinTwitter = define("oni.social.joinTwitter", Command(JoinTwitter));

  let joinDiscord = define("oni.social.joinDiscord", Command(JoinDiscord));

  let joinDevTalk = define("oni.social.joinDevTalk", Command(JoinDevTalk));

  let all = [joinDevTalk, joinTwitter, joinDiscord];
};

module MenuItems = {
  open MenuBar.Schema;
  open Commands;

  let joinDevTalk = command(~title="Join Devtalk", joinDevTalk);
  let joinTwitter = command(~title="Join Twitter", joinTwitter);
  let joinDiscord = command(~title="Join Discord", joinDiscord);

  let socialGroup =
    group(
      ~order=100,
      ~parent=Feature_MenuBar.Global.help,
      [joinDevTalk, joinTwitter, joinDiscord],
    );
};

module Contributions = {
  let menuGroups = MenuItems.[socialGroup];

  let commands = Commands.all;
};
