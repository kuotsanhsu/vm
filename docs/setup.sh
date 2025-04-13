mkdir ~/Documents/vm && code "$_"
# > Git: Initialize Repository
# > Publish to GitHub
git init

{
  # --silent --show-error --location --write-out
  curl -sSLw "\n### End of %{urle.path}\n\n" \
    https://raw.githubusercontent.com/github/gitignore/refs/heads/main/\
{C,Global/{VisualStudioCode,macOS}}.gitignore
  echo main
} > .gitignore

code --install-extension llvm-vs-code-extensions.vscode-clangd

cat > Makefile << EOF
.DEFAULT_GOAL := main
EOF

# For our simple program, make and clangd works right out-of-the box without further configurations.

make && ./make # or, equivalently, `make sign run`
make sign run # sign first then run
make run # no need to sign again for future runs

git_export() {
  # https://stackoverflow.com/a/46176822
  # https://stackoverflow.com/a/79130092
  local -r ext=tar.gz dir="$(basename "$PWD")"
  git archive --format=$ext --output="$dir.$ext" --prefix="$dir/" HEAD
}
git_export

remote_run() {
  scp main sehors:/tmp 
  ssh sehors /tmp/main
}
remote_run
