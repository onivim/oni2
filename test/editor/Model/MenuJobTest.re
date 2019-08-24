open TestFramework;

open Oni_Core;

module MenuJob = Oni_Model.MenuJob;
module Actions = Oni_Model.Actions;

describe("MenuJob", ({describe, _}) => {
  describe("regexFromFilter", ({test, _}) => 
    test("regex matches as expected", ({expect, _}) => {
      let regEx = MenuJob.regexFromFilter("abc");
      expect.bool(Str.string_match(regEx, "abc", 0)).toBe(true);
      expect.bool(Str.string_match(regEx, "def", 0)).toBe(false);
      expect.bool(Str.string_match(regEx, "1a1b1c1", 0)).toBe(true);
    })
  )
  describe("boundary cases", ({test, _}) => {
    test("large amount of items added work", ({expect, _}) => {
      let job = MenuJob.create();
              
              let commands: list(Actions.menuCommand) = List.init(1000000, (i) => {
                          let ret: Actions.menuCommand = {
                         category: None,
                         name: "Item " ++ string_of_int(i),
                         command: () => Oni_Model.Actions.ShowNotification(Oni_Model.Notification.create(~title="derp", ~message=string_of_int(i), ())),
                         icon: None
                      };
                      ret;

              });
  
        let job = Job.map(MenuJob.addItems(commands), job);

        expect.bool(Job.isComplete(job)).toBe(false);
    })
  })
});
