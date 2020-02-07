open TestFramework;

module Notifications = Oni_Model.Notifications;
module Notification = Oni_Model.Notification;

describe("Notifications", ({describe, _}) =>
  describe("getOldestNotification", ({test, _}) =>
    test("retrieves oldest notification", ({expect, _}) => {
      open Oni_Model.Actions;

      let notification1 = Notification.create("a");
      let notification2 = Notification.create("b");

      let state = [];
      let state =
        Notifications.reduce(state, ShowNotification(notification1));
      let state =
        Notifications.reduce(state, ShowNotification(notification2));

      let oldest = Notifications.getOldest(state);

      expect.same(oldest, notification1);
    })
  )
);
