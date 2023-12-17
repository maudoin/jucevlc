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

    juce_add_binary_data(jucevlcIcons
    HEADER_NAME Icons.h
    NAMESPACE Icons
    SOURCES
        ../icons/vlc.png
        ../icons/disk.svg
        ../icons/harddisk.svg
        ../icons/left.svg
        ../icons/right.svg
        ../icons/usb.svg
        ../icons/AlessioAtzeni/add-all.svg
        ../icons/AlessioAtzeni/arrow-reset.svg
        ../icons/AlessioAtzeni/atom.svg
        ../icons/AlessioAtzeni/backCircle.svg
        ../icons/AlessioAtzeni/blank.svg
        ../icons/AlessioAtzeni/clock.svg
        ../icons/AlessioAtzeni/frontpage.svg
        ../icons/AlessioAtzeni/fullscreen.svg
        ../icons/AlessioAtzeni/hide-open.svg
        ../icons/AlessioAtzeni/image.svg
        ../icons/AlessioAtzeni/like-add.svg
        ../icons/AlessioAtzeni/like-remove.svg
        ../icons/AlessioAtzeni/off.svg
        ../icons/AlessioAtzeni/open-menu.svg
        ../icons/AlessioAtzeni/open-short.svg
        ../icons/AlessioAtzeni/options-settings.svg
        ../icons/AlessioAtzeni/pause.svg
        ../icons/AlessioAtzeni/play.svg
        ../icons/AlessioAtzeni/playlist.svg
        ../icons/AlessioAtzeni/reset.svg
        ../icons/AlessioAtzeni/sliders.svg
        ../icons/AlessioAtzeni/sound-on.svg
        ../icons/AlessioAtzeni/sound-shift.svg
        ../icons/AlessioAtzeni/speed.svg
        ../icons/AlessioAtzeni/stop.svg
        ../icons/AlessioAtzeni/subtitles.svg
        ../icons/AlessioAtzeni/window.svg)

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
