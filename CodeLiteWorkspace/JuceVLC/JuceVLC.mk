##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=JuceVLC
ConfigurationName      :=Debug
WorkspacePath          := "E:\dev\vlcfrontend\CodeLiteWorkspace"
ProjectPath            := "E:\dev\vlcfrontend\CodeLiteWorkspace\JuceVLC"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=M
Date                   :=27/09/2013
CodeLitePath           :="E:\dev\CodeLite"
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)JUCE_MINGW 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="JuceVLC.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=windres
LinkOptions            :=  -mwindows
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../src $(IncludeSwitch)../../libvlc/include $(IncludeSwitch)../../libvlc/include/vlc $(IncludeSwitch)../../libvlc/include/vlc/plugins $(IncludeSwitch)../../vf $(IncludeSwitch)../../juce $(IncludeSwitch)../../boost/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)vlc $(LibrarySwitch)vlccore $(LibrarySwitch)boost_regex-vc100-mt-s-1_43 $(LibrarySwitch)imm32 $(LibrarySwitch)glu32 $(LibrarySwitch)shell32 $(LibrarySwitch)ole32 $(LibrarySwitch)ws2_32 $(LibrarySwitch)imm32 $(LibrarySwitch)winmm $(LibrarySwitch)version $(LibrarySwitch)wininet $(LibrarySwitch)opengl32 $(LibrarySwitch)uuid $(LibrarySwitch)oleaut32 $(LibrarySwitch)shlwapi 
ArLibs                 :=  "vlc" "vlccore" "boost_regex-vc100-mt-s-1_43" "libimm32" "libglu32" "libshell32" "libole32" "libws2_32" "libimm32" "libwinmm" "libversion" "libwininet" "libopengl32" "libuuid" "liboleaut32" "libshlwapi" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../../libvlc/lib $(LibraryPathSwitch)../../boost/lib 

##
## Common variables
## AR, CXX, CC, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -g -O0 -Wall -fpermissive $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)


##
## User defined environment variables
##
CodeLiteDir:=E:\dev\CodeLite
UNIT_TEST_PP_SRC_DIR:=e:\dev\UnitTest++-1.3
Objects0=$(IntermediateDirectory)/src_AppProportionnalComponent$(ObjectSuffix) $(IntermediateDirectory)/src_ControlComponent$(ObjectSuffix) $(IntermediateDirectory)/src_Execute$(ObjectSuffix) $(IntermediateDirectory)/src_FileSorter$(ObjectSuffix) $(IntermediateDirectory)/src_FontResource$(ObjectSuffix) $(IntermediateDirectory)/src_FontSerialization$(ObjectSuffix) $(IntermediateDirectory)/src_IconMenu$(ObjectSuffix) $(IntermediateDirectory)/src_Icons$(ObjectSuffix) $(IntermediateDirectory)/src_ImageCatalog$(ObjectSuffix) $(IntermediateDirectory)/src_ImageCatalogCache$(ObjectSuffix) \
	$(IntermediateDirectory)/src_Languages$(ObjectSuffix) $(IntermediateDirectory)/src_Main$(ObjectSuffix) $(IntermediateDirectory)/src_MenuBase$(ObjectSuffix) $(IntermediateDirectory)/src_MenuComponent$(ObjectSuffix) $(IntermediateDirectory)/src_MenuTree$(ObjectSuffix) $(IntermediateDirectory)/src_PosterFinder$(ObjectSuffix) $(IntermediateDirectory)/src_Thumbnailer$(ObjectSuffix) $(IntermediateDirectory)/src_VideoComponent$(ObjectSuffix) $(IntermediateDirectory)/src_VLCWrapper$(ObjectSuffix) $(IntermediateDirectory)/src_resources.rc$(ObjectSuffix) \
	

Objects1=$(IntermediateDirectory)/juce_core_juce_core$(ObjectSuffix) $(IntermediateDirectory)/juce_data_structures_juce_data_structures$(ObjectSuffix) $(IntermediateDirectory)/juce_events_juce_events$(ObjectSuffix) $(IntermediateDirectory)/juce_graphics_juce_graphics$(ObjectSuffix) $(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(ObjectSuffix) $(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(ObjectSuffix) $(IntermediateDirectory)/vf_concurrent_vf_concurrent$(ObjectSuffix) $(IntermediateDirectory)/vf_core_vf_core$(ObjectSuffix) 



Objects=$(Objects0) $(Objects1) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	@echo $(Objects1) >> $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_AppProportionnalComponent$(ObjectSuffix): ../../src/AppProportionnalComponent.cpp $(IntermediateDirectory)/src_AppProportionnalComponent$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/AppProportionnalComponent.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_AppProportionnalComponent$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_AppProportionnalComponent$(DependSuffix): ../../src/AppProportionnalComponent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_AppProportionnalComponent$(ObjectSuffix) -MF$(IntermediateDirectory)/src_AppProportionnalComponent$(DependSuffix) -MM "../../src/AppProportionnalComponent.cpp"

$(IntermediateDirectory)/src_AppProportionnalComponent$(PreprocessSuffix): ../../src/AppProportionnalComponent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_AppProportionnalComponent$(PreprocessSuffix) "../../src/AppProportionnalComponent.cpp"

$(IntermediateDirectory)/src_ControlComponent$(ObjectSuffix): ../../src/ControlComponent.cpp $(IntermediateDirectory)/src_ControlComponent$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/ControlComponent.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ControlComponent$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ControlComponent$(DependSuffix): ../../src/ControlComponent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ControlComponent$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ControlComponent$(DependSuffix) -MM "../../src/ControlComponent.cpp"

$(IntermediateDirectory)/src_ControlComponent$(PreprocessSuffix): ../../src/ControlComponent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ControlComponent$(PreprocessSuffix) "../../src/ControlComponent.cpp"

$(IntermediateDirectory)/src_Execute$(ObjectSuffix): ../../src/Execute.cpp $(IntermediateDirectory)/src_Execute$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/Execute.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Execute$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Execute$(DependSuffix): ../../src/Execute.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Execute$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Execute$(DependSuffix) -MM "../../src/Execute.cpp"

$(IntermediateDirectory)/src_Execute$(PreprocessSuffix): ../../src/Execute.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Execute$(PreprocessSuffix) "../../src/Execute.cpp"

$(IntermediateDirectory)/src_FileSorter$(ObjectSuffix): ../../src/FileSorter.cpp $(IntermediateDirectory)/src_FileSorter$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/FileSorter.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_FileSorter$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_FileSorter$(DependSuffix): ../../src/FileSorter.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_FileSorter$(ObjectSuffix) -MF$(IntermediateDirectory)/src_FileSorter$(DependSuffix) -MM "../../src/FileSorter.cpp"

$(IntermediateDirectory)/src_FileSorter$(PreprocessSuffix): ../../src/FileSorter.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_FileSorter$(PreprocessSuffix) "../../src/FileSorter.cpp"

$(IntermediateDirectory)/src_FontResource$(ObjectSuffix): ../../src/FontResource.cpp $(IntermediateDirectory)/src_FontResource$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/FontResource.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_FontResource$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_FontResource$(DependSuffix): ../../src/FontResource.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_FontResource$(ObjectSuffix) -MF$(IntermediateDirectory)/src_FontResource$(DependSuffix) -MM "../../src/FontResource.cpp"

$(IntermediateDirectory)/src_FontResource$(PreprocessSuffix): ../../src/FontResource.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_FontResource$(PreprocessSuffix) "../../src/FontResource.cpp"

$(IntermediateDirectory)/src_FontSerialization$(ObjectSuffix): ../../src/FontSerialization.cpp $(IntermediateDirectory)/src_FontSerialization$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/FontSerialization.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_FontSerialization$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_FontSerialization$(DependSuffix): ../../src/FontSerialization.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_FontSerialization$(ObjectSuffix) -MF$(IntermediateDirectory)/src_FontSerialization$(DependSuffix) -MM "../../src/FontSerialization.cpp"

$(IntermediateDirectory)/src_FontSerialization$(PreprocessSuffix): ../../src/FontSerialization.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_FontSerialization$(PreprocessSuffix) "../../src/FontSerialization.cpp"

$(IntermediateDirectory)/src_IconMenu$(ObjectSuffix): ../../src/IconMenu.cpp $(IntermediateDirectory)/src_IconMenu$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/IconMenu.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_IconMenu$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_IconMenu$(DependSuffix): ../../src/IconMenu.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_IconMenu$(ObjectSuffix) -MF$(IntermediateDirectory)/src_IconMenu$(DependSuffix) -MM "../../src/IconMenu.cpp"

$(IntermediateDirectory)/src_IconMenu$(PreprocessSuffix): ../../src/IconMenu.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_IconMenu$(PreprocessSuffix) "../../src/IconMenu.cpp"

$(IntermediateDirectory)/src_Icons$(ObjectSuffix): ../../src/Icons.cpp $(IntermediateDirectory)/src_Icons$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/Icons.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Icons$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Icons$(DependSuffix): ../../src/Icons.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Icons$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Icons$(DependSuffix) -MM "../../src/Icons.cpp"

$(IntermediateDirectory)/src_Icons$(PreprocessSuffix): ../../src/Icons.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Icons$(PreprocessSuffix) "../../src/Icons.cpp"

$(IntermediateDirectory)/src_ImageCatalog$(ObjectSuffix): ../../src/ImageCatalog.cpp $(IntermediateDirectory)/src_ImageCatalog$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/ImageCatalog.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ImageCatalog$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ImageCatalog$(DependSuffix): ../../src/ImageCatalog.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ImageCatalog$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ImageCatalog$(DependSuffix) -MM "../../src/ImageCatalog.cpp"

$(IntermediateDirectory)/src_ImageCatalog$(PreprocessSuffix): ../../src/ImageCatalog.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ImageCatalog$(PreprocessSuffix) "../../src/ImageCatalog.cpp"

$(IntermediateDirectory)/src_ImageCatalogCache$(ObjectSuffix): ../../src/ImageCatalogCache.cpp $(IntermediateDirectory)/src_ImageCatalogCache$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/ImageCatalogCache.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ImageCatalogCache$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ImageCatalogCache$(DependSuffix): ../../src/ImageCatalogCache.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ImageCatalogCache$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ImageCatalogCache$(DependSuffix) -MM "../../src/ImageCatalogCache.cpp"

$(IntermediateDirectory)/src_ImageCatalogCache$(PreprocessSuffix): ../../src/ImageCatalogCache.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ImageCatalogCache$(PreprocessSuffix) "../../src/ImageCatalogCache.cpp"

$(IntermediateDirectory)/src_Languages$(ObjectSuffix): ../../src/Languages.cpp $(IntermediateDirectory)/src_Languages$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/Languages.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Languages$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Languages$(DependSuffix): ../../src/Languages.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Languages$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Languages$(DependSuffix) -MM "../../src/Languages.cpp"

$(IntermediateDirectory)/src_Languages$(PreprocessSuffix): ../../src/Languages.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Languages$(PreprocessSuffix) "../../src/Languages.cpp"

$(IntermediateDirectory)/src_Main$(ObjectSuffix): ../../src/Main.cpp $(IntermediateDirectory)/src_Main$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/Main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Main$(DependSuffix): ../../src/Main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Main$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Main$(DependSuffix) -MM "../../src/Main.cpp"

$(IntermediateDirectory)/src_Main$(PreprocessSuffix): ../../src/Main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Main$(PreprocessSuffix) "../../src/Main.cpp"

$(IntermediateDirectory)/src_MenuBase$(ObjectSuffix): ../../src/MenuBase.cpp $(IntermediateDirectory)/src_MenuBase$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/MenuBase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_MenuBase$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_MenuBase$(DependSuffix): ../../src/MenuBase.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_MenuBase$(ObjectSuffix) -MF$(IntermediateDirectory)/src_MenuBase$(DependSuffix) -MM "../../src/MenuBase.cpp"

$(IntermediateDirectory)/src_MenuBase$(PreprocessSuffix): ../../src/MenuBase.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_MenuBase$(PreprocessSuffix) "../../src/MenuBase.cpp"

$(IntermediateDirectory)/src_MenuComponent$(ObjectSuffix): ../../src/MenuComponent.cpp $(IntermediateDirectory)/src_MenuComponent$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/MenuComponent.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_MenuComponent$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_MenuComponent$(DependSuffix): ../../src/MenuComponent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_MenuComponent$(ObjectSuffix) -MF$(IntermediateDirectory)/src_MenuComponent$(DependSuffix) -MM "../../src/MenuComponent.cpp"

$(IntermediateDirectory)/src_MenuComponent$(PreprocessSuffix): ../../src/MenuComponent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_MenuComponent$(PreprocessSuffix) "../../src/MenuComponent.cpp"

$(IntermediateDirectory)/src_MenuTree$(ObjectSuffix): ../../src/MenuTree.cpp $(IntermediateDirectory)/src_MenuTree$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/MenuTree.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_MenuTree$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_MenuTree$(DependSuffix): ../../src/MenuTree.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_MenuTree$(ObjectSuffix) -MF$(IntermediateDirectory)/src_MenuTree$(DependSuffix) -MM "../../src/MenuTree.cpp"

$(IntermediateDirectory)/src_MenuTree$(PreprocessSuffix): ../../src/MenuTree.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_MenuTree$(PreprocessSuffix) "../../src/MenuTree.cpp"

$(IntermediateDirectory)/src_PosterFinder$(ObjectSuffix): ../../src/PosterFinder.cpp $(IntermediateDirectory)/src_PosterFinder$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/PosterFinder.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_PosterFinder$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_PosterFinder$(DependSuffix): ../../src/PosterFinder.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_PosterFinder$(ObjectSuffix) -MF$(IntermediateDirectory)/src_PosterFinder$(DependSuffix) -MM "../../src/PosterFinder.cpp"

$(IntermediateDirectory)/src_PosterFinder$(PreprocessSuffix): ../../src/PosterFinder.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_PosterFinder$(PreprocessSuffix) "../../src/PosterFinder.cpp"

$(IntermediateDirectory)/src_Thumbnailer$(ObjectSuffix): ../../src/Thumbnailer.cpp $(IntermediateDirectory)/src_Thumbnailer$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/Thumbnailer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Thumbnailer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Thumbnailer$(DependSuffix): ../../src/Thumbnailer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Thumbnailer$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Thumbnailer$(DependSuffix) -MM "../../src/Thumbnailer.cpp"

$(IntermediateDirectory)/src_Thumbnailer$(PreprocessSuffix): ../../src/Thumbnailer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Thumbnailer$(PreprocessSuffix) "../../src/Thumbnailer.cpp"

$(IntermediateDirectory)/src_VideoComponent$(ObjectSuffix): ../../src/VideoComponent.cpp $(IntermediateDirectory)/src_VideoComponent$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/VideoComponent.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_VideoComponent$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_VideoComponent$(DependSuffix): ../../src/VideoComponent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_VideoComponent$(ObjectSuffix) -MF$(IntermediateDirectory)/src_VideoComponent$(DependSuffix) -MM "../../src/VideoComponent.cpp"

$(IntermediateDirectory)/src_VideoComponent$(PreprocessSuffix): ../../src/VideoComponent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_VideoComponent$(PreprocessSuffix) "../../src/VideoComponent.cpp"

$(IntermediateDirectory)/src_VLCWrapper$(ObjectSuffix): ../../src/VLCWrapper.cpp $(IntermediateDirectory)/src_VLCWrapper$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/src/VLCWrapper.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_VLCWrapper$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_VLCWrapper$(DependSuffix): ../../src/VLCWrapper.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_VLCWrapper$(ObjectSuffix) -MF$(IntermediateDirectory)/src_VLCWrapper$(DependSuffix) -MM "../../src/VLCWrapper.cpp"

$(IntermediateDirectory)/src_VLCWrapper$(PreprocessSuffix): ../../src/VLCWrapper.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_VLCWrapper$(PreprocessSuffix) "../../src/VLCWrapper.cpp"

$(IntermediateDirectory)/src_resources.rc$(ObjectSuffix): ../../src/resources.rc
	$(RcCompilerName) -i "E:/dev/vlcfrontend/src/resources.rc" $(RcCmpOptions)   $(ObjectSwitch)$(IntermediateDirectory)/src_resources.rc$(ObjectSuffix) $(RcIncludePath)
$(IntermediateDirectory)/juce_core_juce_core$(ObjectSuffix): ../../juce/modules/juce_core/juce_core.cpp $(IntermediateDirectory)/juce_core_juce_core$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/juce/modules/juce_core/juce_core.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/juce_core_juce_core$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/juce_core_juce_core$(DependSuffix): ../../juce/modules/juce_core/juce_core.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/juce_core_juce_core$(ObjectSuffix) -MF$(IntermediateDirectory)/juce_core_juce_core$(DependSuffix) -MM "../../juce/modules/juce_core/juce_core.cpp"

$(IntermediateDirectory)/juce_core_juce_core$(PreprocessSuffix): ../../juce/modules/juce_core/juce_core.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/juce_core_juce_core$(PreprocessSuffix) "../../juce/modules/juce_core/juce_core.cpp"

$(IntermediateDirectory)/juce_data_structures_juce_data_structures$(ObjectSuffix): ../../juce/modules/juce_data_structures/juce_data_structures.cpp $(IntermediateDirectory)/juce_data_structures_juce_data_structures$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/juce/modules/juce_data_structures/juce_data_structures.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/juce_data_structures_juce_data_structures$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/juce_data_structures_juce_data_structures$(DependSuffix): ../../juce/modules/juce_data_structures/juce_data_structures.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/juce_data_structures_juce_data_structures$(ObjectSuffix) -MF$(IntermediateDirectory)/juce_data_structures_juce_data_structures$(DependSuffix) -MM "../../juce/modules/juce_data_structures/juce_data_structures.cpp"

$(IntermediateDirectory)/juce_data_structures_juce_data_structures$(PreprocessSuffix): ../../juce/modules/juce_data_structures/juce_data_structures.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/juce_data_structures_juce_data_structures$(PreprocessSuffix) "../../juce/modules/juce_data_structures/juce_data_structures.cpp"

$(IntermediateDirectory)/juce_events_juce_events$(ObjectSuffix): ../../juce/modules/juce_events/juce_events.cpp $(IntermediateDirectory)/juce_events_juce_events$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/juce/modules/juce_events/juce_events.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/juce_events_juce_events$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/juce_events_juce_events$(DependSuffix): ../../juce/modules/juce_events/juce_events.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/juce_events_juce_events$(ObjectSuffix) -MF$(IntermediateDirectory)/juce_events_juce_events$(DependSuffix) -MM "../../juce/modules/juce_events/juce_events.cpp"

$(IntermediateDirectory)/juce_events_juce_events$(PreprocessSuffix): ../../juce/modules/juce_events/juce_events.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/juce_events_juce_events$(PreprocessSuffix) "../../juce/modules/juce_events/juce_events.cpp"

$(IntermediateDirectory)/juce_graphics_juce_graphics$(ObjectSuffix): ../../juce/modules/juce_graphics/juce_graphics.cpp $(IntermediateDirectory)/juce_graphics_juce_graphics$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/juce/modules/juce_graphics/juce_graphics.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/juce_graphics_juce_graphics$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/juce_graphics_juce_graphics$(DependSuffix): ../../juce/modules/juce_graphics/juce_graphics.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/juce_graphics_juce_graphics$(ObjectSuffix) -MF$(IntermediateDirectory)/juce_graphics_juce_graphics$(DependSuffix) -MM "../../juce/modules/juce_graphics/juce_graphics.cpp"

$(IntermediateDirectory)/juce_graphics_juce_graphics$(PreprocessSuffix): ../../juce/modules/juce_graphics/juce_graphics.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/juce_graphics_juce_graphics$(PreprocessSuffix) "../../juce/modules/juce_graphics/juce_graphics.cpp"

$(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(ObjectSuffix): ../../juce/modules/juce_gui_basics/juce_gui_basics.cpp $(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/juce/modules/juce_gui_basics/juce_gui_basics.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(DependSuffix): ../../juce/modules/juce_gui_basics/juce_gui_basics.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(ObjectSuffix) -MF$(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(DependSuffix) -MM "../../juce/modules/juce_gui_basics/juce_gui_basics.cpp"

$(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(PreprocessSuffix): ../../juce/modules/juce_gui_basics/juce_gui_basics.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(PreprocessSuffix) "../../juce/modules/juce_gui_basics/juce_gui_basics.cpp"

$(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(ObjectSuffix): ../../juce/modules/juce_gui_extra/juce_gui_extra.cpp $(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/juce/modules/juce_gui_extra/juce_gui_extra.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(DependSuffix): ../../juce/modules/juce_gui_extra/juce_gui_extra.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(ObjectSuffix) -MF$(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(DependSuffix) -MM "../../juce/modules/juce_gui_extra/juce_gui_extra.cpp"

$(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(PreprocessSuffix): ../../juce/modules/juce_gui_extra/juce_gui_extra.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(PreprocessSuffix) "../../juce/modules/juce_gui_extra/juce_gui_extra.cpp"

$(IntermediateDirectory)/vf_concurrent_vf_concurrent$(ObjectSuffix): ../../vf/modules/vf_concurrent/vf_concurrent.cpp $(IntermediateDirectory)/vf_concurrent_vf_concurrent$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/vf/modules/vf_concurrent/vf_concurrent.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/vf_concurrent_vf_concurrent$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/vf_concurrent_vf_concurrent$(DependSuffix): ../../vf/modules/vf_concurrent/vf_concurrent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/vf_concurrent_vf_concurrent$(ObjectSuffix) -MF$(IntermediateDirectory)/vf_concurrent_vf_concurrent$(DependSuffix) -MM "../../vf/modules/vf_concurrent/vf_concurrent.cpp"

$(IntermediateDirectory)/vf_concurrent_vf_concurrent$(PreprocessSuffix): ../../vf/modules/vf_concurrent/vf_concurrent.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/vf_concurrent_vf_concurrent$(PreprocessSuffix) "../../vf/modules/vf_concurrent/vf_concurrent.cpp"

$(IntermediateDirectory)/vf_core_vf_core$(ObjectSuffix): ../../vf/modules/vf_core/vf_core.cpp $(IntermediateDirectory)/vf_core_vf_core$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/dev/vlcfrontend/vf/modules/vf_core/vf_core.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/vf_core_vf_core$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/vf_core_vf_core$(DependSuffix): ../../vf/modules/vf_core/vf_core.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/vf_core_vf_core$(ObjectSuffix) -MF$(IntermediateDirectory)/vf_core_vf_core$(DependSuffix) -MM "../../vf/modules/vf_core/vf_core.cpp"

$(IntermediateDirectory)/vf_core_vf_core$(PreprocessSuffix): ../../vf/modules/vf_core/vf_core.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/vf_core_vf_core$(PreprocessSuffix) "../../vf/modules/vf_core/vf_core.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/src_AppProportionnalComponent$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_AppProportionnalComponent$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_AppProportionnalComponent$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_ControlComponent$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_ControlComponent$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_ControlComponent$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_Execute$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_Execute$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_Execute$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_FileSorter$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_FileSorter$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_FileSorter$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_FontResource$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_FontResource$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_FontResource$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_FontSerialization$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_FontSerialization$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_FontSerialization$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_IconMenu$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_IconMenu$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_IconMenu$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_Icons$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_Icons$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_Icons$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_ImageCatalog$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_ImageCatalog$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_ImageCatalog$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_ImageCatalogCache$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_ImageCatalogCache$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_ImageCatalogCache$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_Languages$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_Languages$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_Languages$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_Main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_Main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_Main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuBase$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuBase$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuBase$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuComponent$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuComponent$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuComponent$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuTree$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuTree$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_MenuTree$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_PosterFinder$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_PosterFinder$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_PosterFinder$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_Thumbnailer$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_Thumbnailer$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_Thumbnailer$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_VideoComponent$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_VideoComponent$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_VideoComponent$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_VLCWrapper$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_VLCWrapper$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_VLCWrapper$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/resources.rc$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/juce_core_juce_core$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/juce_core_juce_core$(DependSuffix)
	$(RM) $(IntermediateDirectory)/juce_core_juce_core$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/juce_data_structures_juce_data_structures$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/juce_data_structures_juce_data_structures$(DependSuffix)
	$(RM) $(IntermediateDirectory)/juce_data_structures_juce_data_structures$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/juce_events_juce_events$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/juce_events_juce_events$(DependSuffix)
	$(RM) $(IntermediateDirectory)/juce_events_juce_events$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/juce_graphics_juce_graphics$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/juce_graphics_juce_graphics$(DependSuffix)
	$(RM) $(IntermediateDirectory)/juce_graphics_juce_graphics$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(DependSuffix)
	$(RM) $(IntermediateDirectory)/juce_gui_basics_juce_gui_basics$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(DependSuffix)
	$(RM) $(IntermediateDirectory)/juce_gui_extra_juce_gui_extra$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/vf_concurrent_vf_concurrent$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/vf_concurrent_vf_concurrent$(DependSuffix)
	$(RM) $(IntermediateDirectory)/vf_concurrent_vf_concurrent$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/vf_core_vf_core$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/vf_core_vf_core$(DependSuffix)
	$(RM) $(IntermediateDirectory)/vf_core_vf_core$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe
	$(RM) "../.build-debug/JuceVLC"


