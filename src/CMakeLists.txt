juce_add_gui_app(jucevlc
    COMPANY_NAME "Matthieu A"
    COMPANY_COPYRIGHT "Matthieu A (2024)"
    COMPANY_WEBSITE "https://github.com/maudoin/jucevlc"
    BUNDLE_ID "com.maudoin.jucevlc"
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

    juce_add_binary_data(jucevlcIcons
    HEADER_NAME Icons.h
    NAMESPACE Icons
    SOURCES
        ../icons/vlc.svg
        ../icons/cancel.svg
        ../icons/left.svg
        ../icons/right.svg
        ../icons/settings.svg
        ../icons/settings_open.svg
        ../icons/check.svg
        ../icons/star.svg
        ../icons/star_outline.svg
        ../icons/pause.svg
        ../icons/play.svg
        ../icons/stop.svg
        ../icons/fullscreen.svg
        ../icons/window.svg
        ../icons/layers.svg
        ../icons/sliders.svg
        ../icons/video.svg
        ../icons/video_checked.svg
        ../icons/audio.svg
        ../icons/subtitles.svg
        ../icons/download.svg
        ../icons/aspect_ratio.svg
        ../icons/font.svg
        ../icons/size.svg
        ../icons/horizontal_offset.svg
        ../icons/vertical_offset.svg
        ../icons/crop.svg
        ../icons/fast_forward.svg
        ../icons/toggle_on.svg
        ../icons/toggle_off.svg
        ../icons/file.svg
        ../icons/folder.svg
        ../icons/exit.svg)

        juce_add_binary_data(jucevlcFonts
            HEADER_NAME Fonts.h
            NAMESPACE Fonts
            SOURCES
                ../fonts/Noto_Sans/NotoSans-Regular.ttf)

target_link_libraries(jucevlc
    PRIVATE
        juce::juce_gui_extra
        jucevlcIcons
        jucevlcFonts
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        libvlc
        libvlccore)

code_sign_target_files(jucevlc)
