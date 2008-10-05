!qbuild:qtopia_project(unittest)
HEADERS += alternativequeries.h currentqueries.h filtercombinations.h
SOURCES += alternativequeries.cpp currentqueries.cpp filtercombinations.cpp tst_performance.cpp
!qbuild:depends(src/libraries/qtopiapim)
