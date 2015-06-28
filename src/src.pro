TEMPLATE = subdirs
SUBDIRS = lib \
    plugins \
    tests

tests.depends = lib plugins
