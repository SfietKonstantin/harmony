TEMPLATE = subdirs
SUBDIRS = lib \
    bin \
    plugins \
    tests

bin.depends = lib plugins
tests.depends = lib plugins
