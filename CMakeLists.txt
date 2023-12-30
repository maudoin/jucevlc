cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Yes this suxx:
set(WINDOWS_KIT_VERS "10.0.22621.0")
set(WINDOWS_10_KIT_DIR "C:/Program Files (x86)/Windows Kits/10")

project(JuceVLC VERSION 0.9.4)

# we use inplace sources with a submodule
add_subdirectory(JUCE)

# we use lib extracted from dlls under windows
# and headers are retrieved from a submodule
add_library(libvlc SHARED IMPORTED)
set_target_properties(libvlc PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/contrib/libvlc/lib64/libvlc.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/vlc/include;${CMAKE_SOURCE_DIR}/vlc/lib"
)

# we use lib extracted from dlls under windows
# and headers are retrieved from a submodule
add_library(libvlccore SHARED IMPORTED)
set_target_properties(libvlccore PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/contrib/libvlc/lib64/libvlccore.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/vlc/include;${CMAKE_SOURCE_DIR}/vlc/lib"
)

######## signer
#MakeCert" -pe -n "CN=MatthieuA" -b 12/12/2023 -e 01/01/2100 -ss my -sr currentuser -a sha256 -sky signature -len 2048 -r "matthieua.cer"
#MakeCert" -pe -n "CN=MatthieuA" -b 12/12/2023 -e 01/01/2100 -eku  1.3.6.1.5.5.7.3.3 -in "matthieua" -is my -ir currentuser -ss my -a sha256 -sky exchange  -sp "Microsoft RSA SChannel Cryptographic Provider" -sy 12 -len 2048 "matthieuasign.cer"

set(SIGN_CERT_FILE "${CMAKE_SOURCE_DIR}/MatthieuA.pfx")

find_program(SIGNTOOL
 signtool.exe
 PATHS
 "${WINDOWS_10_KIT_DIR}/bin/${WINDOWS_KIT_VERS}/x64"
)
mark_as_advanced(
  SIGNTOOL
)

function(code_sign_target_files SIGN_TGT_NAME)
  if (${SIGNTOOL} STREQUAL "SIGNTOOL-NOTFOUND")
    message("Missing signtool")
  else()
    if(DEFINED ENV{SIGN_PASSWORD})
        ADD_CUSTOM_COMMAND(
          DEPENDS ${SIGN_TGT_NAME}
          COMMAND $<IF:$<CONFIG:Release>,${SIGNTOOL},echo>
          ARGS $<$<CONFIG:Debug>:"Skipping signing"> sign /debug /a /f ${SIGN_CERT_FILE} /p $ENV{SIGN_PASSWORD} /fd sha256 /tr http://timestamp.digicert.com /td sha256 /v $<TARGET_FILE:${SIGN_TGT_NAME}>
          TARGET ${SIGN_TGT_NAME})
    else()
        message("define SIGN_PASSWORD env variable to sign the executable")
    endif()
  endif()
endfunction()

add_subdirectory(src)

