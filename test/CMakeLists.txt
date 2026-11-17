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
