# Cross toolchain and clangd setup on macOS

```sh
brew tap messense/macos-cross-toolchains
brew install x86_64-unknown-linux-gnu
x86_64-unknown-linux-gnu-gcc -print-search-dirs
# install: /opt/homebrew/Cellar/x86_64-unknown-linux-gnu/13.3.0/toolchain/lib/gcc/x86_64-unknown-linux-gnu/13.3.0/
```

## clangd-17

- linux/fs.h not found in linux/io_uring.h
  - https://clangd.llvm.org/faq#how-do-i-fix-errors-i-get-when-opening-headers-outside-of-my-project-directory
- `--query-driver` does not work with fallback flags for `.h` file if driver is gcc and objective-c++ support is not installed [#1568](https://github.com/clangd/clangd/issues/1568)
  - Specify the clangd argument `--compile-commands-dir=${workspaceFolder}/io_uring` in `settings.json`, as the clangd argument `--query-driver=/opt/homebrew/Cellar/x86_64-unknown-linux-gnu/13.3.0/toolchain/bin/x86_64-linux-gnu-gcc` doesn't seem to work.
- > Main file cannot be included recursively when building a preamble
  - /Library/Developer/CommandLineTools/usr/lib/clang/17/include/stdatomic.h
  - `-resource-dir=/Library/Developer/CommandLineTools/usr/lib/clang/17`
  - To override macOS `stdatomic.h`, specify an `-isystem=/opt/homebrew/Cellar/x86_64-unknown-linux-gnu/13.3.0/toolchain/lib/gcc/x86_64-unknown-linux-gnu/13.3.0/include`
    - > Address argument to atomic operation must be a pointer to a trivially-copyable type (`_Atomic(typeof (*(cring_head))) *` invalid) `clang(atomic_op_needs_trivial_copy)`
      - Is disallowing loading an _Atomic(...) type with __atomic_load intended? [#60572](https://github.com/llvm/llvm-project/issues/60572)

## Update clangd

- Setting ` "clangd.checkUpdates": true` in `settings.json` would yield this on startup:
  > Failed to check for clangd update: Error: Cannot compare vendor's clangd version: Apple clangd version 17.0.0 (clang-1700.0.13.3) Features: mac+xpc Platform: arm64-apple-darwin24.4.0
- `brew search clangd` won't find clangd; one must do `brew install llvm`.
  - Setting `"clangd.path": "/opt/homebrew/opt/llvm/bin/clangd"` in `settings.json` would yield this on startup:
    > clangd is up-to-date (you have 20.1.3, latest is 19.1.2)

## clangd-20

- Like clangd-17, external headers don't fall back to options extracted from `--query-driver`.
- Like clangd-17, `stdatomic.h` from x86_64-unknown-linux-gnu still gives the same error `clang(atomic_op_needs_trivial_copy)`.
