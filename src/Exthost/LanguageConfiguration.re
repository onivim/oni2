
module OnEnterRule = {
   type t = { 
    beforeText: OnigRegExp.t,
    afterText: option(OnigRegExp.t),
    previousLineText: option(OnigRegExp.t),
    action
   }
}
