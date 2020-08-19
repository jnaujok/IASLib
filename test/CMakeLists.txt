cmake_minimum_required(VERSION 3.10)

#set project name
project(IASLibTests VERSION 1.0)

configure_file(TestXMLDocument/DatabaseLayout.xml DatabaseLayout.xml COPYONLY)

#Set up test programs
add_executable(TestXMLDocument TestXMLDocument/TestXMLDocument.cpp)
add_test(test_xml TestXMLDocument)

target_link_libraries(TestXMLDocument IASLib)
