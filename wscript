# -*- python -*-

# special variables
top = "."
out = "build"

# subdirectories
sourcedir = "src"
testdir   = "test"
subdirs   = " ".join([sourcedir, testdir])

# cxx flags
cxxflags = ["-O2", "-Wall", "-g", "-pipe"]

def options(opt):
    opt.load("unittestt", tooldir=testdir)
    opt.load("compiler_cxx")
    opt.recurse(subdirs)

def configure(conf):
    conf.load("compiler_cxx")
    conf.env.append_value("CXXFLAGS", cxxflags)
    conf.load("unittestt", tooldir=testdir)
    conf.check_cxx(lib="gtest_main", uselib_store="GTEST_MAIN")
    conf.recurse(subdirs)

def build(bld):
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
                  """cd src/currentia/query/; \
                  re2c -i -o lexer.h lexer.re""")

def generate_parser(bld):
    __run_command(bld,
                  """cd src/currentia/query/; \
                  ./tools/lemon/lemon cpl-parser.y T=./tools/lemon/lempar-currentia.c; \
                  indent -kr -ts4 --no-tabs ./cpl-parser.c; \
                  ./tools/generate_token_to_string.rb < ./cpl-parser.h > ./cpl-token-to-string.h""")

def generate_lexer_parser(bld):
    generate_parser(bld)
    generate_lexer(bld)
