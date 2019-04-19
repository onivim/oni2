/*
 * ConfigurationParser.rei
 *
 * Resilient parsing for Configuration
 */

let ofJson: Yojson.Safe.json => result(Configuration.t, string);

let ofString: string => result(Configuration.t, string);

let ofFile: string => result(Configuration.t, string);
