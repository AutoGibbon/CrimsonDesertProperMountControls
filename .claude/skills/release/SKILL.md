---
name: release
description: Release a new version of Proper Horse Controls (HorseSteer) - version bump, changelog, README and mod page, pack, release commit, tag and push, then record the Nexus upload. Use when the user asks to release, cut, tag or pack a version, or says a version is uploaded.
---

# Release Proper Horse Controls

Every feature going into the release must be `done` in `docs/state/tasks.json`, with its tests in
`docs/state/tests.json`. If one isn't, say which and stop.

## Steps

1. **Release item:** agree with the user a `release-X-Y-Z` item in `docs/state/tasks.json`, `in_progress`, with
   `depends_on` listing the features it ships and `done_when` "tagged, packed with `tools/pack.py`, uploaded to Nexus".
2. **Version:** set `X.Y.Z` in both `kVersion` in `src/HorseSteer/HorseSteer.cpp` and `project(HorseSteer VERSION ...)`
   in `src/HorseSteer/CMakeLists.txt`. `tools/pack.py` names the zip from `kVersion`, and the log prints it, so a
   mismatch gives a zip whose name differs from what players' logs report.
3. **Changelog:** add `Version X.Y.Z` at the top of `mod_page/changelog.txt`, in plain developer wording, matching the
   most recent entry's style.
4. **Player docs:** if features, settings or compatibility changed, update `README.md` and `mod_page/description.bbcode`.
   Read the BBCode first and match its voice. Credit any new outside source in both and in `docs/discovery.md`
   Resources, and add any compiled-in third-party code to `THIRD_PARTY_NOTICES.txt`.
5. **Pack:** run `python tools/pack.py`. It builds and writes `build/ProperHorseControls-X.Y.Z.zip` with `Logging=0`.
6. **State:** in `docs/state/environment.json`, add the zip as an artefact with the release's date. Leave the release
   item `in_progress`, since upload is still to come.
7. **Commit, tag, push:** commit everything as `Release X.Y.Z: <one-line summary>`, tag that commit `X.Y.Z`, and push
   the commit and the tag. Don't add a follow-up commit recording the tag.
8. **Upload:** the user uploads the zip to Nexus. When they say it's up, set the release item `done` with the tag's
   commit and "user <date>: on Nexus" as evidence, set the zip artefact's `commit` to the tag's commit and
   `nexus_version` under `facts` in `environment.json`, and commit.

If the build deployed in DMM should be the release build, deploy it as `CLAUDE.md` says and update its artefacts in
`environment.json`.
