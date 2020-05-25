/**
 * Oni_Store
 *
 * The Store library is responsible for managing changes in the Model over time,
 * and across integrations. The Model in Oni_Model is abstract, this creates a
 * concrete instance and handles actions and effects.
 *
 * Any change that impacts the state of the editor should be modelled as an Action
 * and funneled through the appropriate reducers / updaters.
 */
module StoreThread = StoreThread;
module Reducer = Reducer;
module Utility = Utility;
module Persistence = Persistence;
