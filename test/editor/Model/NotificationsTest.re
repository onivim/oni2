open TestFramework;

module Notifications = Oni_Model.Notifications;
module Notification = Oni_Model.Notification;

describe("Notifications", ({describe, _}) =>
  describe("getOldestNotification", ({test, _}) =>
    test("retrieves oldest notification", ({expect, _}) => {
      open Notification;
      open Oni_Model.Actions;

      let notification1 = Notification.create(~message="a", ());
      let notification2 = Notification.create(~message="b", ());

      let n1_id = notification1.id;
      let n2_id = notification2.id;

      let state = []
          |> Notifications.reduce(ShowNotification(notification1))
          |> Notifications.reduce(ShowNotification(notification2));

      let oldest = Notification.getOldestId(state);

      expect.int(oldest).toBe(n1_id);
      
    })
  )
);
