open TestFramework;

module Notifications = Oni_Model.Notifications;
module Notification = Oni_Model.Notification;

describe("Notifications", ({describe, _}) =>
  describe("getOldestNotification", ({test, _}) => {
    test("retrieves oldest notification", ({expect, _}) => {
      open Oni_Model.Actions;

      let title = "testMsg";
      let notification1 = Notification.create(~title,~message="a", ());
      let notification2 = Notification.create(~title,~message="b", ());

      let n1_id = notification1.id;
      //let n2_id = notification2.id;

      let state = [];
      let state = Notifications.reduce(state, ShowNotification(notification1));
      let state = Notifications.reduce(state, ShowNotification(notification2));

      let oldest = Notifications.getOldestId(state);

      expect.int(oldest).toBe(n1_id);
      
    });
  })
);
