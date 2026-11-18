# Working on this repository

## Standing rules

- **Update README.md before merging.** Every change that adds or alters a
  feature, a behaviour, an API, a build option, a dependency, benchmark
  figures or a known limitation updates README.md (and CHANGELOG.md) in the
  same pull request, before it is merged. If nothing in the README needs to
  change, say so in the pull request.

## Checks

- `cmake -B build && cmake --build build --parallel && ctest --test-dir build --output-on-failure`
