# -*- python -*-

# special variables
top = "."
out = "build"

# subdirectories
sourcedir = "src"
testdir   = "test"
subdirs   = " ".join([sourcedir, testdir])

# cxx flags
cxxflags = ["-Wall", "-g", "-pipe",
            "-std=gnu++0x",
            "-DCURRENTIA_ENABLE_TRANSACTION",
            "-DCURRENTIA_CHECK_STATISTICS",
            "-O3",
            # "-O0",
            # "-DCURRENTIA_DEBUG"
            # "-DCURRENTIA_ENABLE_TIME_BASED_WINDOW"
            ]


def options(opt):
    opt.load("unittestt", tooldir=testdir)
    opt.load("compiler_cxx")
    opt.recurse(subdirs)

def configure(conf):
    conf.load("compiler_cxx")
    conf.env.append_value("CXXFLAGS", cxxflags)
    conf.load("unittestt", tooldir=testdir)
#     conf.check_cxx(lib="gtest_main", uselib_store="GTEST_MAIN")
    conf.check_cxx(lib="pthread", uselib_store="PTHREAD")
    conf.recurse(subdirs)

def build(bld):
    generate_lexer_parser(bld)
    bld.recurse(subdirs)

def __run_command(bld, cmd):
    try:
        import waflib
        (out, err) = bld.cmd_and_log(cmd, output=waflib.Context.BOTH)
        if out:
            print(out)
        if err:
            print(err)
    except Exception as e:
        print(e)

# src/currentia/query/tools/lemon/

def generate_lexer(bld):
    __run_command(bld,
                  """
cd src/currentia/query/;
re2c -s --no-generation-date -o cpl-lexer.h -i cpl-lexer.re
""")

def generate_parser(bld):
    __run_command(bld,
                  """
cd src/currentia/query/;
lemon cpl-parser.y T=./tools/lemon/lempar-currentia.c;
./tools/format_code.sh ./cpl-parser.c;
./tools/generate_token_to_string.rb < ./cpl-parser.h > ./cpl-token-to-string.h
""")

def generate_lexer_parser(bld):
    generate_lexer(bld)
    generate_parser(bld)
