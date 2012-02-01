# -*- python -*-

subdirs = "src"

def options(opt):
    opt.load("compiler_cxx waf_unit_test")

def configure(conf):
    conf.load("compiler_cxx waf_unit_test")
    conf.env.append_value("CXXFLAGS", ["-O2", "-Wall", "-g", "-pipe"]) # stores configuration persistently (conf.env). See build/c4che/_cache.py
    conf.check_cxx(lib = "gtest_main", uselib_store = "gtest_main")
    # recursively
    conf.recurse(subdirs)

def build(bld):
    from waflib.Tools import waf_unit_test
    bld.add_post_fun(waf_unit_test.summary)
    bld.recurse(subdirs)
