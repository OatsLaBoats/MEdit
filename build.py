from pybaker import *
import sys
import subprocess


def build():
    builder = Builder("MEdit", build_type=BuildType.DEBUG)

    if len(sys.argv) == 2:
        if sys.argv[1] == "clean":
            builder.clean_all()
            return

        if sys.argv[1] == "run":
            subprocess.run(["./build/debug/MEdit.exe"])
            return

    language = Language(
        {".c"}, 
        DependencyScannerC(), 
        CompilerClangC([
            "-Iexternal/nuklear/include", 
            "-Wno-unused-command-line-argument", 
            "-Wno-gnu-zero-variadic-macro-arguments",
            "-std=c17",
        ])
    )

    builder.add_language(language)

    builder.add_path("src")
    builder.add_path("src/app")
    builder.add_path("src/core")
    builder.add_path("src/gui")
    builder.add_path("external/nuklear/src", ["-D_CRT_SECURE_NO_WARNINGS"])

    builder.build()
    builder.link(["-lUser32.lib", "-lGdi32.lib", "-lMsimg32.lib"])


if __name__ == "__main__":
    build()