juce_add_gui_app(jucevlc
    COMPANY_NAME "Matthieu A"
    BUNDLE_ID "com.matthieuaudoin.jucevlc"
    PRODUCT_NAME "JuceVLC"
    ICON_BIG                        ${CMAKE_CURRENT_SOURCE_DIR}/../icons/vlc.png
    NEEDS_CURL                      FALSE
    NEEDS_WEB_BROWSER               FALSE
    MICROPHONE_PERMISSION_ENABLED   FALSE
    CAMERA_PERMISSION_ENABLED       FALSE
    BLUETOOTH_PERMISSION_ENABLED    FALSE
    FILE_SHARING_ENABLED            FALSE
    DOCUMENT_BROWSER_ENABLED        FALSE
    REQUIRES_FULL_SCREEN            FALSE)
juce_generate_juce_header(jucevlc)

target_compile_definitions(jucevlc
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:jucevlc,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:jucevlc,JUCE_VERSION>"
        JUCE_MODAL_LOOPS_PERMITTED=1)


FILE(GLOB CPP_FILES *.cpp)


target_sources(jucevlc
    PRIVATE
        ${CPP_FILES}
    )

# Extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

# juce_add_binary_data(jucevlc SOURCES ...)


target_link_libraries(jucevlc
    PRIVATE
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        libvlc
        libvlccore)


#pack_target(jucevlc)

code_sign_target_files(jucevlc)
