# -*- python -*-

subdirs = "src"

def options(opt):
    opt.load("compiler_cxx")

def configure(conf):
    conf.load("compiler_cxx")
    conf.env.CXXFLAGS += ["-O2", "-Wall", "-g", "-pipe"]
    # recursively
    conf.recurse(subdirs)

def build(bld):
    bld.recurse(subdirs)
