with import <nixpkgs> {};
mkShell {
  buildInputs = [
    libyaml
    gcc
  ];
  shellHook = ''
    export PROMPT_COMMAND=""
    alias compile="gcc -I ./include src/main.c -o gifts -lyaml"
  '';
}
