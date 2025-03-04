with import <nixpkgs> {};
mkShell {
  buildInputs = [
    libyaml
    gcc
  ];
  shellHook = ''
    export PROMPT_COMMAND=""
    alias compile="gcc src/main.c src/tree_from_yaml.c -lyaml -o gifts"
  '';
}
