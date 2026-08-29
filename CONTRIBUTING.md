# Contributing to bitwise

Patches are welcome. This file is the short version of what makes them
easy to merge.

## Getting a tree that builds

You need libreadline, libncurses (with forms), and libcunit for the
tests. On Debian/Ubuntu:

```sh
sudo apt-get install build-essential libncurses-dev libreadline-dev libcunit1-dev
```

The README has the equivalent for macOS and the other platforms. From a
git checkout - as opposed to a release tarball - you have to generate
the build system first:

```sh
./bootstrap.sh
./configure
make
```

## Before you send the patch

Run the tests:

```sh
make check
```

Anything that touches expression parsing, number formatting, or the bit
manipulation itself should come with a test in
`tests/test-shunting-yard.c`. That file is where the regressions we have
already paid for are pinned; adding to it is the cheapest thing you can
do for the next person.

For anything non-trivial, build with the sanitizers on and run the tests
again. They catch the class of bug that is invisible in a normal build:

```sh
./configure --enable-asan --enable-ubsan --enable-debug
make check
```

The expression evaluator also has a libFuzzer harness. It needs clang,
and it is worth a few minutes if you changed the tokenizer or a number
parser:

```sh
./configure CC=clang --enable-fuzzer --enable-asan --enable-ubsan
make tests/fuzz-shunting-yard
mkdir -p corpus && ./tests/fuzz-shunting-yard -max_len=256 corpus
```

Builds should be warning-free. `-Wall -Wextra` is on by default, so a
new warning in your patch will be visible in CI.

## Style

C code follows the Linux kernel style: tabs for indentation, 80 columns
where it doesn't hurt readability, and `snake_case` names. The
`.editorconfig` in the repo covers the mechanical part, so most editors
get it right without being told.

## Commits and pull requests

* One logical change per commit. A fix and the refactor that made room
  for it are two commits.
* Write the commit message for someone reading it in two years with no
  memory of the discussion: what was wrong, and why this is the fix.
  Subject line in the imperative, under ~72 characters.
* Rebase on current `master` rather than merging it into your branch.
* CI runs the tests on Linux, macOS and Windows/msys2, plus a
  `make distcheck` and a sanitizer build. All of it should be green
  before you ask for a review.

## Reporting bugs

Open an issue with the expression or the key sequence that triggers it,
what you expected, and what you got. `bitwise -v` and your OS help. If
it is a crash, a build with `--enable-debug --enable-asan` and the
resulting report is worth more than any description.
