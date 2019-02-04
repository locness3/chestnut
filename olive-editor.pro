# Tries to get the current Git short hash
system("which git") {
    GITHASHVAR = $$system(git --git-dir $$PWD/.git --work-tree $$PWD log -1 --format=%h)
    DEFINES += GITHASH=\\"\"$$GITHASHVAR\\"\"
}

TEMPLATE = subdirs

SUBDIRS = app

unix:CONFIG(unittests) {
    SUBDIRS += unittests
} else {
    SUBDIRS += app/main
}


CONFIG += ordered
