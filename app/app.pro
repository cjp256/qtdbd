TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = \
    dbd \
    db-cat \
    db-dump \
    db-exists \
    db-inject \
    db-ls \
    db-nodes \
    db-read \
    db-rm \
    db-write

dbd.depends = src
db-cat.depends = src
db-dump.depends = src
db-exists.depends = src
db-inject.depends = src
db-ls.depends = src
db-nodes.depends = src
db-read.depends = src
db-rm.depends = src
db-write.depends = src
