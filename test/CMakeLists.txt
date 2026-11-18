cmake_minimum_required(VERSION 3.10)

#set project name
project(IASLibTests VERSION 1.0)

configure_file(TestXMLDocument/DatabaseLayout.xml DatabaseLayout.xml COPYONLY)

#Set up test programs
add_executable(TestXMLDocument TestXMLDocument/TestXMLDocument.cpp)
add_test(test_xml TestXMLDocument)

target_link_libraries(TestXMLDocument IASLib)

add_executable(TestSip TestSip/TestSip.cpp)
target_link_libraries(TestSip IASLib)
add_test(test_sip TestSip)

add_executable(TestHttp TestHttp/TestHttp.cpp)
target_link_libraries(TestHttp IASLib)
add_test(test_http TestHttp)

add_executable(TestDate TestDate/TestDate.cpp)
target_link_libraries(TestDate IASLib)
add_test(test_date TestDate)

if(USE_JSON)
    add_executable(TestJson TestJson/TestJson.cpp)
    target_link_libraries(TestJson IASLib)
    add_test(test_json TestJson)
endif()

if(USE_NETWORK AND OPENSSL_FOUND)
    add_executable(TestTls TestTls/TestTls.cpp)
    target_link_libraries(TestTls IASLib)
    add_test(test_tls TestTls)
endif()

if(USE_JSON)
    add_executable(TestBlockchain TestBlockchain/TestBlockchain.cpp)
    target_link_libraries(TestBlockchain IASLib)
    add_test(test_blockchain TestBlockchain)
endif()

add_executable(TestBaseTypes TestBaseTypes/TestBaseTypes.cpp)
target_link_libraries(TestBaseTypes IASLib)
add_test(test_base_types TestBaseTypes)

add_executable(TestCollections TestCollections/TestCollections.cpp)
target_link_libraries(TestCollections IASLib)
add_test(test_collections TestCollections)

if(USE_THREADS)
    add_executable(TestThreading TestThreading/TestThreading.cpp)
    target_link_libraries(TestThreading IASLib)
    add_test(test_threading TestThreading)
endif()

add_executable(TestStreams TestStreams/TestStreams.cpp)
target_link_libraries(TestStreams IASLib)
add_test(test_streams TestStreams)

add_executable(TestSocketClasses TestSocketClasses/TestSocketClasses.cpp)
target_link_libraries(TestSocketClasses IASLib)
add_test(test_socket_classes TestSocketClasses)

add_executable(TestStats TestStats/TestStats.cpp)
target_link_libraries(TestStats IASLib)
add_test(test_stats TestStats)

add_executable(TestLogging TestLogging/TestLogging.cpp)
target_link_libraries(TestLogging IASLib)
add_test(NAME test_logging COMMAND TestLogging WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(TestMemory TestMemory/TestMemory.cpp)
target_link_libraries(TestMemory IASLib)
add_test(test_memory TestMemory)

add_executable(TestResources TestResources/TestResources.cpp)
target_link_libraries(TestResources IASLib)
add_test(NAME test_resources COMMAND TestResources WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
