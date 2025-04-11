with import <nixpkgs> {};
mkShell {
  buildInputs = [
    libyaml
    gcc
    openssl
  ];
  shellHook = ''
    export PROMPT_COMMAND=""
    alias compile="gcc -I ./include -I src/main.c src/serialize/serialize.c src/serialize/header.c src/parsers/yaml_parser.c -o gifts -lyaml -lssl -lcrypto"  '';
}

# alias compile="gcc -I ./include src/main.c src/serializer/serialize.c -o gifts -lyaml"
