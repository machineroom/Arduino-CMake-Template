
set(SOURCE_FILES
    iclient.cpp
)
include_directories(../WIZ ../WIZ/utility ./)

arduino(iclient ${SOURCE_FILES} WIZ)
