set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../source/types/CircList.cpp
    ../../source/types/List.cpp
    ../../source/types/Cib.cpp
    ../../source/core/MicroByteCpu.cpp
    ../../source/core/MicroByteThread.cpp
    ../../source/core/MicroByteMutex.cpp
    ../../source/core/MicroByteMsg.cpp
    ../../source/core/MicroByteEvent.cpp
    stubs/MicroByteCpuTest.cpp
)

set(unittest-test-sources
    source/core/MicroByteEvent/test_MicroByteEvent.cpp
)
