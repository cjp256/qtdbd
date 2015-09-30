TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = \
    qtdbd-unittests \
    dbd-perftest

qtdbd-unittests.depends = src
dbd-perftest.depends = src
