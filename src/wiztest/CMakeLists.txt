
set(SOURCE_FILES
    wiztest.cpp
)
include_directories(../WIZ ./)

arduino(wiztest ${SOURCE_FILES} WIZ)
