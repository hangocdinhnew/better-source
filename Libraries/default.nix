{
  pkgs ? import <nixpkgs> { },
}:
with pkgs;

let
    buildDeps = with pkgs; [
        clang-tools
        cmake
        pkg-config
        ninja
        cppcheck
        doxygen
        makeWrapper
    ];

    runtimeDeps = with pkgs; [
        sdl3
        freetype
        fontconfig
        openal
        libjpeg
        curl
        bzip2
        libedit
    ];

    libraryPath = pkgs.lib.makeLibraryPath runtimeDeps;
in
mkShell.override { } {
    nativeBuildInputs = with pkgs; [
    ] ++ buildDeps;

    buildInputs = with pkgs; [
    ] ++ runtimeDeps;

    inputsFrom = with pkgs; [
    ] ++ runtimeDeps;

    shellHook = ''
        if [ "$(uname)" = "Darwin" ]; then
            export DYLD_LIBRARY_PATH=${libraryPath}:$DYLD_LIBRARY_PATH
        else
            export LD_LIBRARY_PATH=${libraryPath}:$LD_LIBRARY_PATH
        fi
    '';
}
