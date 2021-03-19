/*
 * LegacyConfigurationParser.rei
 *
 * Resilient parsing for Configuration
 *
 * This module is DEPRECATED and should not be added or extended
 *
 */

let ofJson: Yojson.Safe.t => result(LegacyConfiguration.t, string);

let ofString: string => result(LegacyConfiguration.t, string);

let ofFile: string => result(LegacyConfiguration.t, string);
