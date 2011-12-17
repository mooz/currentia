# -*- python -*-

subdirs = "src"

def options(opt):
    opt.load("compiler_cxx")

def configure(conf):
    conf.load("compiler_cxx")
    conf.env.CXXFLAGS += ["-O2", "-Wall", "-g"]
    # check
    # conf.check_cxx(header_name = 'unistd.h')
    # conf.check_cxx(header_name = 'signal.h')
    # recursively
    conf.recurse(subdirs)

def build(bld):
    bld.recurse(subdirs)
