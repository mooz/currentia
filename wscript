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

def gen_lexer(bld):
    try:
        import waflib
        (out, err) = bld.cmd_and_log(
            're2c -i -o src/currentia/query/lexer.h src/currentia/query/lexer.re',
            output=waflib.Context.BOTH
        )
        if out:
            print(out)
        if err:
            print(err)
    except Exception as e:
        if e.stdout:
            print(e.stdout)
        if e.stderr:
            print(e.stderr)

