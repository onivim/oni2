# Onivim 2 Release Process

## `stable` releases

The first week of every month - we'll cut a `stable` release.

- Kick-off a `staging` build to get the latest staging packages
- Run manual test pass
    - Create a branch off staging - `release/x.x.x/test-pass`
    - Copy `manual_test/cases.md` to `staging` branch as `STABLE_SIGNOFF.md`
    - Run cases and update `STABLE_SIGNOFF.md`
- Once `staging` passes:
    - `git checkout stable`
    - `git reset --hard staging`
    - Update `package.json` to reflect new version
    - `git commit -m "vx.x.x"`
    - Update `stable`: `git push --force origin stable`
    - Create tag: `git tag vx.x.x`
    - Push tag: `git push vx.x.x`
- Draft release notes
    - Pull from `CHANGES_CURRENT.md` and screenshots via `S-screenshot` label
- Draft video

## `staging` releases

In the middle of every month, we'll cut a `staging` release. This is
to minimize churn prior to a `stable` release.

- Create `staging` branch
  - Force-push `staging` to point to latest master
  - Update `package.json` to point to correct version
  - Copy `manual_test/cases.md` to `STAGING_SIGNOFF.md`
  - Kick off a `staging` build to get last staging packages
  - Run test cases as `STAGING_SIGNOFF.md`
  
 - Update `master`
   - Copy `CHANGES_CURRENT.md` over to `CHANGES.md`

If any regressions are identified, or crash/daily editor blocker fixes go in,
they can be cherry-picked from `master` -> `staging`.

The idea is we can push out monthly releases with a low regression rate,
with the downside that `stable` builds are between 0.5 to 1.5 months behind 
`master`.
