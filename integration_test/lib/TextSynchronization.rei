/*
 * TextSynchronization.rei
 *
 * Helper utility for integration tests to verify that the text
 * across various sources (Onivim, Vim, and ExtHost) are in sync.
 */

/* [validateTextIsSynchronized] takes a dispatch and a wait function,
      and validates that the text across all sources is synchronized.
   */
let validateTextIsSynchronized:
  (
    ~expectedText: option(string)=?,
    dispatchFunction,
    waitForState,
    ~description: string
  ) =>
  unit;
