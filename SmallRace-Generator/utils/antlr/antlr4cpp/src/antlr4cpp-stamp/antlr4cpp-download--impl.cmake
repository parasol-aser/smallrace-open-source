set(command "/usr/local/bin/cmake;-P;/git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-stamp/verify-antlr4cpp.cmake")

execute_process(COMMAND ${command} RESULT_VARIABLE result)
if(result)
  set(msg "Command failed (${result}):\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  message(FATAL_ERROR "${msg}")
endif()
set(command "/usr/local/bin/cmake;-P;/git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-stamp/extract-antlr4cpp.cmake")

execute_process(COMMAND ${command} RESULT_VARIABLE result)
if(result)
  set(msg "Command failed (${result}):\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  message(FATAL_ERROR "${msg}")
endif()