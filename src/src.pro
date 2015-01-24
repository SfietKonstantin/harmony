TEMPLATE = subdirs
SUBDIRS = lib bin tests

tests.depends = lib
bin.depends = lib
